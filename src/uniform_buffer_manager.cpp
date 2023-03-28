#include "uniform_buffer_manager.h"

IVRUBManager::IVRUBManager(VkDevice logical_device, VkPhysicalDevice physical_device, uint16_t frames_in_flight) :
    LogicalDevice_{logical_device}, PhysicalDevice_{physical_device}, FramesInFlight_{frames_in_flight}
{
}

void IVRUBManager::CreateUniformBuffers()
{
    VkDeviceSize buffer_size = sizeof(MVPUniformBufferObject);

    UniformBuffers.resize(FramesInFlight_); //MAX_FRAMES_IN_FLIGHT
    UniformBuffersMemory.resize(FramesInFlight_);
    UniformBuffersMapped.resize(FramesInFlight_);

    for(size_t i = 0; i < FramesInFlight_; i++)
    {
        IVRBufferUtilities::Spawn(
            LogicalDevice_, PhysicalDevice_,
            buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            UniformBuffers[i], UniformBuffersMemory[i]);
        
        vkMapMemory(LogicalDevice_, UniformBuffersMemory[i], 0, buffer_size, 0, &UniformBuffersMapped[i]);
        //the buffer is now mapped to a memory that the host can write to
        //this mapping will stay for the application's lifetime
        //this kind of mapping is called persistent mapping
    }
}

void IVRUBManager::DestroyUniformBuffers()
{
    for(size_t i = 0; i < FramesInFlight_; i++)
    {
        vkDestroyBuffer(LogicalDevice_, UniformBuffers[i], nullptr);
        vkFreeMemory(LogicalDevice_, UniformBuffersMemory[i], nullptr);
    }
}
