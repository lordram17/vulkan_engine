#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <cstring>


#include "buffer_utils.h"
#include "device_setup.h"

class IVRUBManager {
private:
    VkDeviceSize BufferSize_;

    std::shared_ptr<IVRDeviceManager> DeviceManager_;

    void CreateUniformBuffer();
    void DestroyUniformBuffer();

public:

    //disable copy constructor and assignment operator
    IVRUBManager(const IVRUBManager&) = delete;

    IVRUBManager(std::shared_ptr<IVRDeviceManager> device_manager, VkDeviceSize buffer_size);
    ~IVRUBManager();

    //Need to have multiple buffers because frames may be in flight at the same time and
    //we dont want to update the buffer in preparation of the next frame while a previous one is still reading from it
    //Thus we need to have as many buffers as we have frames in flight, and write to a uniform buffer that is not currently being read by the GPU
    VkBuffer UniformBuffer;
    VkDeviceMemory UniformBuffersMemory;
    void* UniformBuffersMapped;

    //call this function to write to the uniform buffer
    void WriteToUniformBuffer(void* source_memory, VkDeviceSize source_object_size);

    VkDeviceSize GetBufferSize();
    VkBuffer GetBuffer();
};