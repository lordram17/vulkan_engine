#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

#include "texture.h"


class IVRDepthImage{

private:
    VkImage DepthImage_;
    VkDeviceMemory DepthImageMemory_;
    VkImageView DepthImageView_;

    VkDevice LogicalDevice_;
    VkPhysicalDevice PhysicalDevice_;
    VkExtent2D DepthImageExtent_;
    uint32_t QueueFamilyIndex_;
    VkQueue Queue_;

public:

    IVRDepthImage(VkDevice logical_device, VkPhysicalDevice physical_device, uint32_t queue_family_index, VkQueue queue,
                 VkExtent2D depth_image_extent);

    void CreateDepthResources();

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features);
    
    VkFormat FindDepthFormat();

    VkImageView GetDepthImageView();

    static bool HasStencilComponent(VkFormat format);

};
