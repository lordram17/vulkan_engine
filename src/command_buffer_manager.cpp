#include "command_buffer_manager.h"

void IVRCBManager::CreateCommandPool(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice logical_device)
{
    QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device, surface);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //allows command buffers to be rerecorded individually, without this flag they all have to be reset together
    //we will record a command buffer every frame, so we want to be able to reset and rerecord over it. Therefore we are using the above bit.
    pool_info.queueFamilyIndex = queue_family_indices.graphicsFamily;

    //Command buffers are executed by submitting them on one of the device queues (like the graphics and presentation queues we retrieved)
    if(vkCreateCommandPool(logical_device, &pool_info, nullptr, &CommandPool_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }

}

void IVRCBManager::DestroyCommandPool(VkDevice logical_device)
{
    vkDestroyCommandPool(logical_device, CommandPool_, nullptr);
}

VkCommandBuffer IVRCBManager::CreateCommandBuffer(VkDevice logical_device)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = CommandPool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //can be submitted to a queue for execution, but cannot be called from other command buffers 
    // (SECONDARY cannot be submitted directly but can be called primary command buffers)
    alloc_info.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(logical_device, &alloc_info, &CommandBuffer_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffer!");
    }

    return CommandBuffer_;
}

void IVRCBManager::RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, 
                                        VkRenderPass renderpass, std::shared_ptr<IVRSwapchainManager> swapchain_manager,
                                        VkPipeline graphics_pipeline, std::shared_ptr<IVRModel> model)
{
    //this function writes the commands we want to execute into a command buffer

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = nullptr; //only relevant for secondary command buffers

    if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer");
    }
    // Not : if the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it
    // it is not possible to append commands to a buffer at a later time

    // STARTING A RENDER PASS
    // drawing starts by beginning the render pass

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = renderpass;
    render_pass_info.framebuffer = swapchain_manager->GetFramebuffer(image_index);

    render_pass_info.renderArea.offset = {0,0};
    render_pass_info.renderArea.extent = swapchain_manager->GetSwapchainExtent();
    //renderArea defines where the shader loads and stores will take place. Pixels outside this region will have undefined values.

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount= 1;
    render_pass_info.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    //since the viewport and scissor were set to be dynamic, we need to specify them here
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain_manager->GetSwapchainExtent().width);
    viewport.height = static_cast<float>(swapchain_manager->GetSwapchainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = swapchain_manager->GetSwapchainExtent();
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer vertex_buffers[] = {model->VertexBuffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    
    vkCmdBindIndexBuffer(command_buffer, model->IndexBuffer_ , 0, VK_INDEX_TYPE_UINT16);


    vkCmdDrawIndexed(CommandBuffer_, static_cast<uint32_t>(model->indices.size()), 1, 0, 0, 0);

    //vkCmdDraw(command_buffer, 3, 1, 0, 0);
    // vertexCount : number of vertices to draw
    // instanceCount : used for instanced rendering, use 1 if not doing that
    // firstVertex : used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex
    // firstInstance : used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex

    vkCmdEndRenderPass(command_buffer);

    if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer");
    }

}
