#pragma once
#include <vector>
#include "simple_file_reader.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "swapchain_manager.h"
#include <memory>

class IVRPipelineManager 
{
private:

    VkDevice LogicalDevice_;
    VkPipelineLayout PipelineLayout_;
    std::shared_ptr<IVRSwapchainManager> SwapchainManager_;
    VkRenderPass RenderPass_;
    VkPipeline Pipeline_;

public:

    IVRPipelineManager(VkDevice logical_device, std::shared_ptr<IVRSwapchainManager> swapchain_manager)
    {
        LogicalDevice_ = logical_device;
        SwapchainManager_ = swapchain_manager;
    }

    VkPipeline CreateGraphicsPipeline();

    void CreateRenderPass();
    VkRenderPass GetRenderPass();

    void DestroyPipelineLayout();
    void DestroyRenderPass();
    void DestroyPipeline();

    VkShaderModule CreateShaderModule(const std::vector<char>& bytecode);


};