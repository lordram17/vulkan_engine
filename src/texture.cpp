#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


IVRTexObj::IVRTexObj(VkDevice logical_device, VkPhysicalDevice physical_device, 
uint32_t queue_family_index, VkQueue queue, const char* texture_path) :
LogicalDevice_{logical_device} , PhysicalDevice_{physical_device}, 
TexturePath_{texture_path}, QueueFamilyIndex_{queue_family_index},
Queue_{queue}
{
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
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

    CreateVkImage(tex_width, tex_height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TextureImage_, TextureImageMemory_);

    //at this point we have the texture data in a staging buffer and we have a texture image
    //The next steps are:
    // 1. Transition the layout of texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // 2. Execute the buffer to image copy operation

    TransitionImageLayout(TextureImage_, VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    CopyBufferToImage(staging_buffer, TextureImage_, 
        static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));

    //To start sampling from the image, we need another layout transition
    TransitionImageLayout(TextureImage_, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(LogicalDevice_, staging_buffer, nullptr);
    vkFreeMemory(LogicalDevice_, staging_buffer_memory, nullptr);

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

VkImageView IVRTexObj::CreateVkImageView(VkDevice logical_device, VkImage image, VkFormat format)
{
    
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    //view_info.components is left out because we want to use the default mapping

    VkImageView image_view;

    if(vkCreateImageView(logical_device, &view_info, nullptr, &image_view) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view");
    }

    return image_view;
}

void IVRTexObj::CreateTextureImageView()
{
    TextureImageView_ = CreateVkImageView(LogicalDevice_, TextureImage_, VK_FORMAT_R8G8B8A8_SRGB);
}

VkImageView IVRTexObj::GetTextureImageView()
{
    return TextureImageView_;
}

void IVRTexObj::CreateTextureSampler()
{
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //following mag and min filters specify how to interpolate texels that are magnified or minified
    //magnification = oversampling, minification = undersampling
    //  magnification means there are more pixels/fragments than texels 
    //  minification means there are fewer pixels/fragments than texels
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    //adress modes specify what to do when sampling beyond the image
    // options : repeat, mirrored repeat, clamp to edge, clamp to border, and mirror clamp to edge
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; //x-axis
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; //y-axis
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; //z-axis

    sampler_info.anisotropyEnable = VK_TRUE; //enable anisotropic filtering
    //also Note : anisotropic filtering is an optional device feature
    //to figure out the max anisotropy, we need to query the device
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(PhysicalDevice_, &physical_device_properties);
    sampler_info.maxAnisotropy = physical_device_properties.limits.maxSamplerAnisotropy;

    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //border color for clamp to border
    sampler_info.unnormalizedCoordinates = VK_FALSE; //if true, texel coordinates are not normalized (0 to 1) but are instead in texel units (0, texture_width/height)
    sampler_info.compareEnable = VK_FALSE; //if enabled, texels will first be compared to a value, and the results will be used in filtering operations
    //compareEnable is mainly used for percentage-closer filtering for shadow maps
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS; //comparison operation to use if compareEnable is true

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; //mipmap mode
    sampler_info.mipLodBias = 0.0f; //bias to use when sampling from a mipmap
    sampler_info.minLod = 0.0f; //minimum level of detail to use when sampling from a mipmap
    sampler_info.maxLod = 0.0f; //maximum level of detail to use when sampling from a mipmap
    //mipmaps are another type of filter that can be applied to textures (will be looked at later)

    if(vkCreateSampler(LogicalDevice_, &sampler_info, nullptr, &TextureSampler_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

VkSampler IVRTexObj::GetTextureSampler()
{
    return TextureSampler_;
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

void IVRTexObj::CopyBufferToImage(VkBuffer src_buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandPool command_pool = CreateCommandPool();
    VkCommandBuffer command_buffer = BeginSingleTimeCommands(command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0,0,0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(command_buffer, src_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(command_buffer, command_pool);
}

void IVRTexObj::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandPool command_pool = CreateCommandPool();
    VkCommandBuffer command_buffer = BeginSingleTimeCommands(command_pool);

    //a common way to perform layout transitions is using an image memory barrier
    //this kind of pipeline barrier like this is generally used to synchronize access to resources
    // like ensuring that a write to buffer is complete before reading from it
    //But it can also be used to transition image layouts and transfer queue family ownership
    // when VK_SHARING_MODE_EXCLUSIVE is used
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //ignored because we dont want to transfer queue family ownership
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    // Note : interestingly imageviewcreateinfo also have subresourceRange with the same fields
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    //Note : In PipelineManager SubpassDependency have the same following two fields
    barrier.srcAccessMask = 0; //To Do
    barrier.dstAccessMask = 0; //To Do

    //There are two transitions that we need to handle
    // 1. Transitioning from undefined to transfer destination
    // 2. Transitioning from transfer destination to shader read : 
    //      shader read should wait on transfer write (specifically the shader read in fragment shader)

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && 
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
        new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(command_buffer, 
        source_stage, 
        destination_stage, 
        0, 0, 
        nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(command_buffer, command_pool);
}

void IVRTexObj::CleanUp()
{
    vkDestroySampler(LogicalDevice_, TextureSampler_, nullptr);
    vkDestroyImageView(LogicalDevice_, TextureImageView_, nullptr);
    vkDestroyImage(LogicalDevice_, TextureImage_, nullptr);
    vkFreeMemory(LogicalDevice_, TextureImageMemory_, nullptr);
}
