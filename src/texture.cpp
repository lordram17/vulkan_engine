#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION


IVRTexture::IVRTexture(std::shared_ptr<IVRDeviceManager> device_manager) :
    DeviceManager_{device_manager}
{
}

IVRTexture::~IVRTexture()
{
    CleanUp();
}



void IVRTexture::CreateTextureImageView()
{
    IVRImageUtils::CreateImageView(DeviceManager_->GetLogicalDevice(), TextureImage_, TextureFormat_, 
                                    VK_IMAGE_ASPECT_COLOR_BIT, ImageViewType_, LayerCount_, TextureImageView_);
}

void IVRTexture::CreateTextureSampler()
{
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    //following mag and min filters specify how to interpolate texels that are magnified or minified
    //magnification = oversampling, minification = undersampling
    //  magnification means there are more pixels/fragments than texels 
    //  minification means there are fewer pixels/fragments than texels

    //if we want to use linear filtering, we check that the texture format supports it
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(DeviceManager_->GetPhysicalDevice(), TextureFormat_, &format_properties);
    if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
    {
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
    else
    {
        sampler_info.magFilter = VK_FILTER_NEAREST;
        sampler_info.minFilter = VK_FILTER_NEAREST;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }

     //mipmap mode
    sampler_info.mipLodBias = 0.0f; //bias to use when sampling from a mipmap
    sampler_info.minLod = 0.0f; //minimum level of detail to use when sampling from a mipmap
    sampler_info.maxLod = 0.0f; //maximum level of detail to use when sampling from a mipmap
    //mipmaps are another type of filter that can be applied to textures (will be looked at later)

    //adress modes specify what to do when sampling beyond the image
    // options : repeat, mirrored repeat, clamp to edge (this is for cubemap), clamp to border, and mirror clamp to edge
    sampler_info.addressModeU = SamplerAddressMode_; //x-axis
    sampler_info.addressModeV = SamplerAddressMode_; //y-axis
    sampler_info.addressModeW = SamplerAddressMode_; //z-axis

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



    if (vkCreateSampler(DeviceManager_->GetLogicalDevice(), &sampler_info, nullptr, &TextureSampler_) != VK_SUCCESS)
    {
        IVR_LOG_ERROR("Failed to create texture sampler");
        throw std::runtime_error("Failed to create texture sampler");
    }
}

void IVRTexture::InitTexture()
{
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
}

VkImage IVRTexture::GetTextureImage()
{
    return TextureImage_;
}


VkSampler IVRTexture::GetTextureSampler()
{
    return TextureSampler_;
}

VkImageView IVRTexture::GetTextureImageView()
{
    return TextureImageView_;
}

void IVRTexture::CleanUp()
{
    vkDestroySampler(DeviceManager_->GetLogicalDevice(), TextureSampler_, nullptr);
    vkDestroyImageView(DeviceManager_->GetLogicalDevice(), TextureImageView_, nullptr);
    vkDestroyImage(DeviceManager_->GetLogicalDevice(), TextureImage_, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), TextureImageMemory_, nullptr);
}
