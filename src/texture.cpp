#include "texture.h"

IVRTexObj::IVRTexObj(VkDevice logical_device, VkPhysicalDevice physical_device, 
uint32_t queue_family_index, VkQueue queue, char* texture_path) :
LogicalDevice_{logical_device} , PhysicalDevice_{physical_device}, 
TexturePath_{texture_path}, QueueFamilyIndex_{queue_family_index},
Queue_{queue}
{
}

void IVRTexObj::CreateTextureImage()
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(TexturePath_, &tex_width, 
        &tex_height, &tex_channels, STBI_rgb_alpha);
    //STBI_rgb_alpha forces the image to be loaded with an alpha channel (this is for consistency between image formats)

    VkDeviceSize image_size = tex_width * tex_height * 4;

    if(!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    IVRBufferUtilities::Spawn(LogicalDevice_, PhysicalDevice_, image_size,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(LogicalDevice_, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(LogicalDevice_, staging_buffer_memory);

    stbi_image_free(pixels); //cleaning up the pixel array

    VkImage texture_image;
    VkDeviceMemory texture_image_memory;

    CreateVkImage(tex_width, tex_height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image, texture_image_memory);

}

void IVRTexObj::CreateVkImage(uint32_t width, uint32_t height, VkFormat format, 
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags, 
    VkImage &image, VkDeviceMemory &image_memory)
{
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;

    image_info.format = format; //need to use the same format for texels as the pixels in the buffer
    image_info.tiling = tiling; //texels are laid out in an implementation defined order for optimal access from the shader

    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
    //there are two possible initial layout values:
    // 1. undefined (like above) - not usable by GPU, first layout transition will discard the texels
    // 2. preinitialized - not usable by GPU, but first layout transition will preserve the texels
    // in our case, we will transition the image to be a transfer destination and then copy texel data to it from a buffer object

    image_info.usage = usage;
    //sampled_bit because we want to access the image from the shader 

    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //image will be used by a single queue family

    image_info.samples = VK_SAMPLE_COUNT_1_BIT; //related to multisampling, only relevant for images that will be used as attachments
    image_info.flags = 0; //some optional flags for images that will be used as sparse images

    if(vkCreateImage(LogicalDevice_, &image_info, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create texture image");
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(LogicalDevice_, image, &memory_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = IVRBufferUtilities::FindMemoryType(PhysicalDevice_, 
    memory_requirements.memoryTypeBits, memory_property_flags);

    if(vkAllocateMemory(LogicalDevice_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory");
    }

    vkBindImageMemory(LogicalDevice_, image, image_memory, 0);
}

VkCommandPool IVRTexObj::CreateCommandPool()
{
    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pool_info.queueFamilyIndex = QueueFamilyIndex_;
    if(vkCreateCommandPool(LogicalDevice_, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool for texture image work");
    }

    return command_pool;
}

VkCommandBuffer IVRTexObj::BeginSingleTimeCommands(VkCommandPool command_pool)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(LogicalDevice_, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void IVRTexObj::EndSingleTimeCommands(VkCommandBuffer command_buffer, VkCommandPool command_pool)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(Queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(Queue_);

    vkFreeCommandBuffers(LogicalDevice_, command_pool, 1, &command_buffer);
    vkDestroyCommandPool(LogicalDevice_, command_pool, nullptr);
}

void IVRTexObj::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize buffer_size)
{
    VkCommandPool command_pool = CreateCommandPool();
    VkCommandBuffer command_buffer = BeginSingleTimeCommands(command_pool);

    VkBufferCopy copy_region{};
    copy_region.size = buffer_size;

    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    EndSingleTimeCommands(command_buffer, command_pool);
}

void IVRTexObj::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandPool command_pool = CreateCommandPool();
    VkCommandBuffer command_buffer = BeginSingleTimeCommands(command_pool);

    EndSingleTimeCommands(command_buffer, command_pool);
}
