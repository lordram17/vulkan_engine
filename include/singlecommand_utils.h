#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

class IVRSingleCommandUtil {
public:
    static VkCommandPool CreateCommandPool(VkDevice logical_device, uint32_t queue_family_index)
    {
        VkCommandPool command_pool;
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool_info.queueFamilyIndex = queue_family_index;
        if (vkCreateCommandPool(logical_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool for texture image work");
        }

        return command_pool;
    }
    
    static VkCommandBuffer BeginSingleTimeCommands(VkDevice logical_device, VkCommandPool command_pool)
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(command_buffer, &begin_info);

        return command_buffer;
    }
    
    static void SubmitAndEndSingleTimeCommands(VkDevice logical_device, VkQueue queue, VkCommandBuffer command_buffer, VkCommandPool command_pool) {
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
        vkDestroyCommandPool(logical_device, command_pool, nullptr);
    }
};

