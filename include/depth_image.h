#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

#include "texture.h"
#include "image_utils.h"
#include "device_setup.h"


class IVRDepthImage{

private:
    VkImage DepthImage_;
    VkDeviceMemory DepthImageMemory_;
    VkImageView DepthImageView_;
    VkExtent2D DepthImageExtent_;
    std::shared_ptr<IVRDeviceManager> DeviceManager_;

public:

    IVRDepthImage(std::shared_ptr<IVRDeviceManager> device_manager, VkExtent2D depth_image_extent);

    void CreateDepthResources();

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features);
    
    VkFormat FindDepthFormat();

    VkImageView GetDepthImageView();

};
