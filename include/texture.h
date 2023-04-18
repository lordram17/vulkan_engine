#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>

#include "buffer_utils.h"
#include "depth_image.h"


class IVRTexObj {

private:

    const char* TexturePath_;
    VkDevice LogicalDevice_;
    VkPhysicalDevice PhysicalDevice_;
    uint32_t QueueFamilyIndex_;
    VkQueue Queue_;
    VkImage TextureImage_;
    VkDeviceMemory TextureImageMemory_;
    VkImageView TextureImageView_;
    VkSampler TextureSampler_;

public:
    IVRTexObj(VkDevice logical_device, VkPhysicalDevice physical_device, uint32_t queue_family_index, VkQueue queue, const char* texture_path);

    void CreateTextureImage();    

    static void CreateVkImage(VkDevice logical_device, VkPhysicalDevice physical_device,
        uint32_t width, uint32_t height, VkFormat format, 
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags,
        VkImage& image, VkDeviceMemory& image_memory);

    static VkImageView CreateVkImageView(VkDevice LogicalDevice_, 
                                        VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);

    void CreateTextureImageView();
    VkImageView GetTextureImageView();

    void CreateTextureSampler();
    VkSampler GetTextureSampler();

    static VkCommandPool CreateCommandPool(VkDevice logical_device, uint32_t queue_family_index);
    static VkCommandBuffer BeginSingleTimeCommands(VkDevice logical_device, VkCommandPool command_pool);
    static void EndSingleTimeCommands(VkDevice logical_device, VkQueue queue, VkCommandBuffer command_buffer, VkCommandPool command_pool);

    void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize buffer_size);
    void CopyBufferToImage(VkBuffer src_buffer, VkImage image, uint32_t width, uint32_t height);

    static void TransitionImageLayout(VkDevice logical_device, uint32_t queue_family_index, VkQueue queue,
        VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

    void CleanUp();

};

