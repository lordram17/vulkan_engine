#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h> // GLFW loads its own definitions and automatically loads the vulkan header with it

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <cstring>
#include <set>
#include <chrono>

#include "instance_setup.h"
#include "device_setup.h"
#include "swapchain_manager.h"
#include "pipeline_manager.h"
#include "command_buffer_manager.h"
#include "camera.h"
#include "uniform_buffer_manager.h"

#define NDEBUG

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;


#ifdef NDEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif




class IvrApp {
public:
    void run();

private:

    GLFWwindow* Window_;
    VkInstance Instance_;
    VkPhysicalDevice PhysicalDevice_ = VK_NULL_HANDLE;
    VkDevice LogicalDevice_;
    
    VkQueue GraphicsQueue_; //queues are automatically created along with the logical device. Also they are implicitly cleaned when the device is destroyed.
    uint32_t GraphicsQueueFamilyIndex_;
    VkQueue PresentQueue_;

    VkCommandBuffer CommandBuffer_;
    VkRenderPass RenderPass_;
    VkPipeline GraphicsPipeline_;

    VkSurfaceKHR Surface_; //rendered images are presented to the vk surface. created by GLFW, thus connected to the glfw window using Window System Integration extensions (like VK_KHR_surface)
    
    std::shared_ptr<IVRSwapchainManager> SwapchainManager_;
    std::shared_ptr<IVRCBManager> CommandBufferManager_;
    std::shared_ptr<IVRPipelineManager> PipelineManager_;
    std::shared_ptr<IVRModel> Model_;
    std::shared_ptr<IVRUBManager> UniformBufferManager_;

    VkSemaphore ImageAvailableSemaphore_;
    VkSemaphore RenderFinishedSemaphore_;
    VkFence InFlightFence_;

    void initWindow() ;

    void initVulkan() ;

    void mainLoop() ;

    void cleanup();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSurface();

    void DrawFrame();

    void CreateSyncObjects();

    void UpdateMVPUniformBuffer(uint32_t current_image_index);

};