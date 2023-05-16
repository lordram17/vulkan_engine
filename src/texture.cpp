#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


IVRTexObj::IVRTexObj(std::shared_ptr<IVRDeviceManager> device_manager, std::string texture_path) :
    DeviceManager_{device_manager}, TexturePath_{texture_path}
{
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
}

void IVRTexObj::CreateTextureImage()
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(TexturePath_.c_str(), &tex_width,
        &tex_height, &tex_channels, STBI_rgb_alpha);
    //STBI_rgb_alpha forces the image to be loaded with an alpha channel (this is for consistency between image formats)

    VkDeviceSize image_size = tex_width * tex_height * 4;

    if(!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

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

    IVRImageUtils::CreateVkImageAndBindMemory(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
    tex_width, tex_height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TextureImage_, TextureImageMemory_);

    //at this point we have the texture data in a staging buffer and we have a texture image
    //The next steps are:
    // 1. Transition the layout of texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // 2. Execute the buffer to image copy operation

    IVRImageUtils::TransitionImageLayout(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        TextureImage_, VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    IVRImageUtils::CopyBufferToImage(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(), 
        staging_buffer, TextureImage_,
        static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));

    //To start sampling from the image, we need another layout transition
    IVRImageUtils::TransitionImageLayout(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        TextureImage_, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), staging_buffer, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, nullptr);
}

void IVRTexObj::CreateTextureImageView()
{
    TextureImageView_ = IVRImageUtils::CreateImageView(DeviceManager_->GetLogicalDevice(), TextureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
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
    vkGetPhysicalDeviceProperties(DeviceManager_->GetPhysicalDevice(), &physical_device_properties);
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

    if (vkCreateSampler(DeviceManager_->GetLogicalDevice(), &sampler_info, nullptr, &TextureSampler_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture sampler");
    }
}


VkSampler IVRTexObj::GetTextureSampler()
{
    return TextureSampler_;
}

VkImageView IVRTexObj::GetTextureImageView()
{
    return TextureImageView_;
}

void IVRTexObj::CleanUp()
{
    vkDestroySampler(DeviceManager_->GetLogicalDevice(), TextureSampler_, nullptr);
    vkDestroyImageView(DeviceManager_->GetLogicalDevice(), TextureImageView_, nullptr);
    vkDestroyImage(DeviceManager_->GetLogicalDevice(), TextureImage_, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), TextureImageMemory_, nullptr);
}
