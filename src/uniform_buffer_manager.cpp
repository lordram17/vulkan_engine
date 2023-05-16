#include "uniform_buffer_manager.h"

IVRUBManager::IVRUBManager(std::shared_ptr<IVRDeviceManager> device_manager, VkDeviceSize buffer_size) :
    DeviceManager_{device_manager}, BufferSize_{buffer_size}
{
    CreateUniformBuffer();
}

IVRUBManager::~IVRUBManager()
{
    DestroyUniformBuffer();
}

void IVRUBManager::CreateUniformBuffer()
{
    IVRBufferUtilities::Spawn(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        BufferSize_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        UniformBuffer, UniformBuffersMemory);
        
    vkMapMemory(DeviceManager_->GetLogicalDevice(), UniformBuffersMemory, 0, BufferSize_, 0, &UniformBuffersMapped);
    //the buffer is now mapped to a memory that the host can write to
    //this mapping will stay for the application's lifetime (Persistent Mapping)
    
}

void IVRUBManager::DestroyUniformBuffer()
{
    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), UniformBuffer, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), UniformBuffersMemory, nullptr);
}

void IVRUBManager::WriteToUniformBuffer(void* source_memory, VkDeviceSize source_object_size)
{
    //before memcpy make sure that the size of the buffer is the same as the size of the source memory

    if (source_object_size != BufferSize_)
    {
		throw std::runtime_error("IVRUBManager::WriteToUniformBuffer: source_object_size != BufferSize_");
	}

    memcpy(UniformBuffersMapped, source_memory, (size_t) BufferSize_);
}

VkDeviceSize IVRUBManager::GetBufferSize()
{
    return BufferSize_;
}

VkBuffer IVRUBManager::GetBuffer()
{
    return UniformBuffer;
}
