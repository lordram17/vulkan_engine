#include "depth_image.h"

IVRDepthImage::IVRDepthImage(std::shared_ptr<IVRDeviceManager> device_manager, VkExtent2D depth_image_extent) :
     DeviceManager_{device_manager}, DepthImageExtent_{ depth_image_extent }
{
    CreateDepthResources();
}

void IVRDepthImage::CreateDepthResources()
{
    //depth image will have same resolution as the color attachment, defined by swap chain extent
    //  image usage should be appropriate for depth attachment
    //  tiling -> optimal, and it should use device local memory
    
    // however, the format of the depth image is still in question
    // there are several formats that can work (depth only needs a single channel float)
    // to select a format, we ideally want to pick one that is supported by the device

    VkFormat depth_format = FindDepthFormat();

    IVRImageUtils::CreateImageAndBindMemory(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(), DepthImageExtent_.width, DepthImageExtent_.height, depth_format, 1,
        VK_IMAGE_TILING_OPTIMAL, 0, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        DepthImage_, DepthImageMemory_);
    
    IVRImageUtils::CreateImageView(DeviceManager_->GetLogicalDevice(), DepthImage_, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, DepthImageView_);
    
    IVRImageUtils::TransitionImageLayout(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        DepthImage_, depth_format, 1,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat IVRDepthImage::FindSupportedFormat_(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    //the support of a format depends on the tiling mode and usage

    for(VkFormat format : candidates)
    {
        VkFormatProperties format_properties;
        
        vkGetPhysicalDeviceFormatProperties(DeviceManager_->GetPhysicalDevice(), format, &format_properties);

        if(tiling == VK_IMAGE_TILING_LINEAR && (format_properties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && (format_properties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat IVRDepthImage::FindDepthFormat()
{
    return FindSupportedFormat_(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkImageView IVRDepthImage::GetDepthImageView()
{
    return DepthImageView_;
}

void IVRDepthImage::TransitionDepthImageToShaderRead()
{
    IVRImageUtils::TransitionImageLayout(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        DepthImage_, FindDepthFormat(), 1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


