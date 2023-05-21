#include "texture_2d.h"



IVRTexture2D::IVRTexture2D(std::shared_ptr<IVRDeviceManager> device_manager, std::string texture_path) :
	IVRTexture(device_manager),
	TexturePath_(texture_path)
{
    ImageViewType_ = VK_IMAGE_VIEW_TYPE_2D;
    SamplerAddressMode_ = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    TextureFormat_ = VK_FORMAT_R8G8B8A8_SRGB;
    InitTexture();
}

IVRTexture2D::~IVRTexture2D() 
{
}

void IVRTexture2D::CreateTextureImage() 
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(TexturePath_.c_str(), &tex_width,
        &tex_height, &tex_channels, STBI_rgb_alpha);
    //STBI_rgb_alpha forces the image to be loaded with an alpha channel (this is for consistency between image formats)

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    VkDeviceSize image_size = tex_width * tex_height * 4; //4 is the number of channels (we are forcing the image to have an alpha channel)

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    IVRBufferUtilities::Spawn(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(), image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory);

    stbi_image_free(pixels); //cleaning up the pixel array

    IVRImageUtils::CreateImageAndBindMemory(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        tex_width, tex_height, TextureFormat_, 1, VK_IMAGE_TILING_OPTIMAL, 0,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TextureImage_, TextureImageMemory_);

    //at this point we have the texture data in a staging buffer and we have a texture image
    //The next steps are:
    // 1. Transition the layout of texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // 2. Execute the buffer to image copy operation

    IVRImageUtils::TransitionImageLayout(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        TextureImage_, TextureFormat_, LayerCount_,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    IVRImageUtils::CopyBufferToImage(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(), 1,
        staging_buffer, TextureImage_, tex_width, tex_height);

    //To start sampling from the image, we need another layout transition
    IVRImageUtils::TransitionImageLayout(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        TextureImage_, TextureFormat_, LayerCount_,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), staging_buffer, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, nullptr);
}


