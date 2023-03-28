#include "model.h"

IVRModel::IVRModel(VkDevice logical_device, VkPhysicalDevice physical_device,
    VkQueue queue, uint32_t queue_family_index) :
    LogicalDevice_{logical_device} , PhysicalDevice_{physical_device},
    Queue_{queue}, QueueFamilyIndex_{queue_family_index}
{
    
}

IVRModel::~IVRModel()
{
    vkDestroyBuffer(LogicalDevice_, VertexBuffer_, nullptr);
    vkFreeMemory(LogicalDevice_, VertexBufferMemory_, nullptr);

    vkDestroyBuffer(LogicalDevice_, IndexBuffer_, nullptr);
    vkFreeMemory(LogicalDevice_, IndexBufferMemory_, nullptr);
}

void IVRModel::CreateVertexBuffer()
{
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    IVRBufferUtilities::Spawn(
        LogicalDevice_, PhysicalDevice_,
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, //buffer can be used as source in memory transfer operation
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_buffer_memory);

    //TRANSFER vertex data from host memory to staging buffer memory
    void* data;
    vkMapMemory(LogicalDevice_, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), (size_t)buffer_size);
    vkUnmapMemory(LogicalDevice_, staging_buffer_memory);

    IVRBufferUtilities::Spawn(
        LogicalDevice_, PhysicalDevice_,
        buffer_size, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VertexBuffer_, VertexBufferMemory_);

    IVRBufferUtilities::TransferBufferData(
        LogicalDevice_, PhysicalDevice_, 
        QueueFamilyIndex_, Queue_,
        staging_buffer, VertexBuffer_, buffer_size);
    
    vkDestroyBuffer(LogicalDevice_, staging_buffer, nullptr);
    vkFreeMemory(LogicalDevice_, staging_buffer_memory, nullptr);
}

void IVRModel::CreateIndexBuffer()
{
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    IVRBufferUtilities::Spawn(LogicalDevice_, PhysicalDevice_,
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_buffer_memory);
    
    void* data;
    vkMapMemory(LogicalDevice_, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), (size_t)buffer_size);
    vkUnmapMemory(LogicalDevice_, staging_buffer_memory);

    IVRBufferUtilities::Spawn(LogicalDevice_, PhysicalDevice_,
        buffer_size, 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        IndexBuffer_, IndexBufferMemory_);
    
    IVRBufferUtilities::TransferBufferData(
        LogicalDevice_, PhysicalDevice_, QueueFamilyIndex_, Queue_,
        staging_buffer, IndexBuffer_, buffer_size);
    
    vkDestroyBuffer(LogicalDevice_, staging_buffer, VK_NULL_HANDLE);
    vkFreeMemory(LogicalDevice_, staging_buffer_memory, VK_NULL_HANDLE);

}

uint32_t IVRModel::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    //graphic cards can offer different types of memory to allocate from
    //each memory varies in terms of allowed operations and performance characteristics
    //we need to find the right type of memory to use

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice_, &memory_properties);

    //memory_properties has two arrays : memoryTypes and memoryHeaps
    // memoryHeaps are distinct memory resources like dedicated VRAM and swap space in RAM (if VRAM runs out)
    // right now we are only concerned with memoryType and not the heap where it comes from

    for(uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        //dont completely understand this if statement yet
        if((type_filter & (1 << i)) && 
        (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0;
}
