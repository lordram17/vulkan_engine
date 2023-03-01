#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h> // GLFW loads its own definitions and automatically loads the vulkan header with it

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <cstring>
#include <set>

#include "instance_setup.h"
#include "device_setup.h"
#include "swapchain_manager.h"
#include "pipeline_manager.h"

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
    VkQueue PresentQueue_;

    VkSurfaceKHR Surface_; //rendered images are presented to the vk surface. created by GLFW, thus connected to the glfw window using Window System Integration extensions (like VK_KHR_surface)
    std::shared_ptr<SwapchainManager> SwapchainManager_;

    void initWindow() ;

    void initVulkan() ;

    void mainLoop() ;

    void cleanup();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSurface();

};