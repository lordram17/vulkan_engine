#include <vector>
#include "simple_file_reader.h"
#include <iostream>
#include <vulkan/vulkan.h>
#include "swapchain_manager.h"
#include <memory>

class PipelineManager 
{
private:

    VkDevice LogicalDevice_;
    VkPipelineLayout PipelineLayout_;
    std::shared_ptr<SwapchainManager> SwapchainManager_;
    VkRenderPass RenderPass_;
    VkPipeline Pipeline_;

public:

    PipelineManager(VkDevice logical_device, std::shared_ptr<SwapchainManager> swapchain_manager)
    {
        LogicalDevice_ = logical_device;
        SwapchainManager_ = swapchain_manager;
    }

    void CreateGraphicsPipeline();

    void CreateRenderPass();

    void DestroyPipelineLayout();
    void DestroyRenderPass();
    void DestroyPipeline();

    VkShaderModule CreateShaderModule(const std::vector<char>& bytecode);


};