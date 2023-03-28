#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include "camera.h"
#include "buffer_utils.h"

class IVRUBManager {
private:
    VkDevice LogicalDevice_;
    VkPhysicalDevice PhysicalDevice_;

    VkDeviceMemory UniformBufferMemory_;
    VkBuffer UniformBuffer_;
    

public:
    IVRUBManager(VkDevice logical_device, VkPhysicalDevice physical_device, uint16_t frames_in_flight);

    //Need to have multiple buffers because frames may be in flight at the same time and
    //we dont want to update the buffer in preparation of the next frame while a previous one is still reading from it
    //Thus we need to have as many buffers as we have frames in flight, and write to a uniform buffer that is not currently being read by the GPU
    std::vector<VkBuffer> UniformBuffers;
    std::vector<VkDeviceMemory> UniformBuffersMemory;
    std::vector<void*> UniformBuffersMapped;
    uint16_t FramesInFlight_; 

    void CreateUniformBuffers();
    void DestroyUniformBuffers();

};