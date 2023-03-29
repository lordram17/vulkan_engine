#include "depth_image.h"

IVRDepthImage::IVRDepthImage(VkDevice logical_device, VkPhysicalDevice physical_device, 
                            uint32_t queue_family_index, VkQueue queue, VkExtent2D depth_image_extent) :
LogicalDevice_{logical_device}, PhysicalDevice_{physical_device}, QueueFamilyIndex_{queue_family_index}, Queue_{queue},
DepthImageExtent_{depth_image_extent}
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

    IVRTexObj::CreateVkImage(
        LogicalDevice_, PhysicalDevice_, DepthImageExtent_.width, DepthImageExtent_.height, depth_format,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        DepthImage_, DepthImageMemory_);
    
    DepthImageView_ = IVRTexObj::CreateVkImageView(LogicalDevice_, DepthImage_, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
    
    IVRTexObj::TransitionImageLayout(
        LogicalDevice_, QueueFamilyIndex_, Queue_,
        DepthImage_, depth_format, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat IVRDepthImage::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    //the support of a format depends on the tiling mode and usage

    for(VkFormat format : candidates)
    {
        VkFormatProperties format_properties;
        
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice_, format, &format_properties);

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
    return FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkImageView IVRDepthImage::GetDepthImageView()
{
    return DepthImageView_;
}

bool IVRDepthImage::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
