#pragma once
#include <vector>
#include "simple_file_reader.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include <memory>

#include "swapchain_manager.h"
#include "model.h"
#include "uniform_buffer_manager.h"


class IVRPipelineManager 
{
private:

    VkDevice LogicalDevice_;
    VkPipelineLayout PipelineLayout_;
    std::shared_ptr<IVRSwapchainManager> SwapchainManager_;
    std::shared_ptr<IVRUBManager> UniformBufferManager_;
    VkRenderPass RenderPass_;
    VkPipeline Pipeline_;

    
    VkDescriptorPool DescriptorPool_;

public:

    IVRPipelineManager(VkDevice logical_device, std::shared_ptr<IVRSwapchainManager> swapchain_manager,
        std::shared_ptr<IVRUBManager> uniform_buffer_manager)
    {
        LogicalDevice_ = logical_device;
        SwapchainManager_ = swapchain_manager;
        UniformBufferManager_ = uniform_buffer_manager;
    }

    std::vector<VkDescriptorSet> DescriptorSets;

    VkPipeline CreateGraphicsPipeline();

    void CreateRenderPass();
    VkRenderPass GetRenderPass();

    void CreateDescriptorPool();
    void CreateDescriptorSets(VkDescriptorSetLayout descriptor_set_layout);
    void DestroyDescriptorPoolAndLayout();

    void DestroyPipelineLayout();
    void DestroyRenderPass();
    void DestroyPipeline();

    VkShaderModule CreateShaderModule(const std::vector<char>& bytecode);
    VkPipelineLayout GetPipelineLayout();


};