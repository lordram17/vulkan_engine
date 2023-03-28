#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>

#include "stb_image.h"
#include "buffer_utils.h"

class IVRTexObj {

private:

    char* TexturePath_;
    VkDevice LogicalDevice_;
    VkPhysicalDevice PhysicalDevice_;
    uint32_t QueueFamilyIndex_;
    VkQueue Queue_;

public:
    IVRTexObj(VkDevice logical_device, VkPhysicalDevice physical_device, uint32_t queue_family_index, VkQueue queue, char* texture_path);

    void CreateTextureImage();    

    void CreateVkImage(uint32_t width, uint32_t height, VkFormat format, 
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags,
        VkImage& image, VkDeviceMemory& image_memory);

    VkCommandPool CreateCommandPool();
    VkCommandBuffer BeginSingleTimeCommands(VkCommandPool command_pool);
    void EndSingleTimeCommands(VkCommandBuffer command_buffer, VkCommandPool command_pool);

    void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize buffer_size);

    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

};