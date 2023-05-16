#pragma once
#include <vulkan/vulkan.h>
#include <memory>

#include "pipeline_manager.h"
#include "device_setup.h"
#include "swapchain_manager.h"
#include "model.h"
#include "debug_logger_utils.h"

class IVRCBManager
{
private:

    VkCommandPool CommandPool_;
    VkCommandBuffer CommandBuffer_;

    std::shared_ptr<IVRDeviceManager> DeviceManager_;

public:

    IVRCBManager(std::shared_ptr<IVRDeviceManager> device_manager);

    void CreateCommandPool();
    void DestroyCommandPool();
    void CreateCommandBuffer();

    //void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, 
    //                        VkRenderPass renderpass, std::shared_ptr<IVRSwapchainManager> swapchain_manager,
    //                        VkPipeline graphics_pipeline, std::shared_ptr<IVRModel> model);

    void StartCommandBuffer();
    void EndCommandBuffer();

    VkCommandBuffer GetCommandBuffer() { return CommandBuffer_; }
};