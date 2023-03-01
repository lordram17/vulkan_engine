
#include "ivr_app.h"


void IvrApp::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void IvrApp::initWindow() 
{
    glfwInit(); //initializes the glfw library
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //glfw was originally designed to create an OpenGL context. This function tells it to not create an OpenGL context.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); //temporarily disabled

    Window_ = glfwCreateWindow(WIDTH, HEIGHT, "IVR", nullptr, nullptr);
}

void IvrApp::initVulkan() {

    IVRInstanceCreator instance_creator(true);
    Instance_ = instance_creator.CreateVulkanInstance();

    createSurface();

    IVRDeviceCreator device_creator;
    PhysicalDevice_ = device_creator.PickPhysicalDevice(Surface_, Instance_);
    LogicalDevice_ = device_creator.CreateLogicalDevice(Surface_);
    GraphicsQueue_ = device_creator.GetGraphicsQueue();
    PresentQueue_ = device_creator.GetPresentQueue();

    SwapchainManager_ = std::make_shared<SwapchainManager>();
    SwapchainManager_->CreateSwapchain(LogicalDevice_, PhysicalDevice_, Surface_, device_creator.GetDeviceQueueFamilies());
    
    PipelineManager pipeline_manager(LogicalDevice_, SwapchainManager_);
    pipeline_manager.CreateGraphicsPipeline();

}

void IvrApp::mainLoop() {
    while(!glfwWindowShouldClose(Window_))
    {
        glfwPollEvents();
    }
}

void IvrApp::cleanup() {
    SwapchainManager_->DestroySwapchain(LogicalDevice_);
    vkDestroySurfaceKHR(Instance_, Surface_, nullptr); //must be destroyed before the instance since it needs the instance
    vkDestroyInstance(Instance_, nullptr);
    glfwDestroyWindow(Window_);
    glfwTerminate();
}



void IvrApp::createSurface()
{
    //there are platform specific Vulkan functions to create the window surface using specific structs (like struct VkWin32SurfaceCreateInfoKHR and function vkCreateWin32SurfaceKHR)
    //but glfw gives us a platform agnostic way of creating the vulkan surface 

    if(glfwCreateWindowSurface(Instance_, Window_, nullptr, &Surface_) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }
}



int main() {
    IvrApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}