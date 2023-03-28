#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "pipeline_manager.h"
#include "device_setup.h"
#include "swapchain_manager.h"
#include "model.h"

class IVRCBManager
{
private:

    VkCommandPool CommandPool_;
    VkCommandBuffer CommandBuffer_;
    
    std::shared_ptr<IVRPipelineManager> PipelineManager_;

public:

    IVRCBManager(std::shared_ptr<IVRPipelineManager> pipeline_manager) :
    PipelineManager_{pipeline_manager}
    {
    }

    void CreateCommandPool(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice logical_device);
    void DestroyCommandPool(VkDevice logical_device);

    VkCommandBuffer CreateCommandBuffer(VkDevice logical_device);
    void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, 
                            VkRenderPass renderpass, std::shared_ptr<IVRSwapchainManager> swapchain_manager,
                            VkPipeline graphics_pipeline, std::shared_ptr<IVRModel> model);

};