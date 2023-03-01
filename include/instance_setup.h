#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h> // GLFW loads its own definitions and automatically loads the vulkan header with it

#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstring>

class IVRInstanceCreator {

private:

    VkInstance Instance_;
    bool EnableValidationLayers_ = true;

    const std::vector<const char*> ValidationLayers_ = {"VK_LAYER_KHRONOS_validation", 
                                                        "VK_LAYER_LUNARG_api_dump"};

public:

    IVRInstanceCreator(bool isEnableValidationLayers);
    ~IVRInstanceCreator();

    VkInstance CreateVulkanInstance();    
    bool CheckValidationLayerSupport();
    VkInstance GetInstance();

};