#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

class IVRBufferUtilities {
public:
    static void Spawn(
        VkDevice logical_device,
        VkPhysicalDevice physical_device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& buffer_memory
    )
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size; //size of the buffer in bytes
        buffer_info.usage = usage; //purpose of this buffer (it is possible to specify multiple usage with bitwise OR)
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //like swapchain images, buffers can also be owned by a specific queue family or shared between multiple
        //this buffer will only be used by the graphics queue, so it is kept exclusive

        if (vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create buffer");
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = FindMemoryType(physical_device, memory_requirements.memoryTypeBits, properties);

        if(vkAllocateMemory(logical_device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(logical_device, buffer, buffer_memory, 0);
    }

    static uint32_t FindMemoryType(
        VkPhysicalDevice physical_device,
        uint32_t type_filter, 
        VkMemoryPropertyFlags properties)
    {
        //graphic cards can offer different types of memory to allocate from
        //each memory varies in terms of allowed operations and performance characteristics
        //we need to find the right type of memory to use

        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

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

    static void TransferBufferData(VkDevice logical_device, VkPhysicalDevice physical_device, uint32_t queue_family_index,
    VkQueue queue, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize buffer_size)
    {
        //can create a separate command pool for this kind of short lived buffer operation
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool_info.queueFamilyIndex = queue_family_index;

        VkCommandPool command_pool;

        if(vkCreateCommandPool(logical_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool for transfer buffer!");
        }

        //transfer memory from one buffer to another locally on the GPU
        //memory transfer operations are executed using command buffers

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //we are only going to submit the command buffer once
        //this will also wait from returning the function until the copy operation is done
        vkBeginCommandBuffer(command_buffer, &begin_info);

        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = buffer_size;
        vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
        vkDestroyCommandPool(logical_device, command_pool, VK_NULL_HANDLE);
    }   

};