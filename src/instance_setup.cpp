#include "instance_setup.h"
#include <iostream>
#include <algorithm>
#include <vector>

IVRInstanceManager::IVRInstanceManager()
{
}

IVRInstanceManager::~IVRInstanceManager()
{
    //vkDestroyInstance(Instance_, nullptr);
}

void IVRInstanceManager::SetAppName(std::string app_name)
{
    AppName_ = app_name;
}

void IVRInstanceManager::EnableValidationLayersForInstanceCreation()
{
    EnableValidationLayers_ = true;
}

VkInstance IVRInstanceManager::CreateVulkanInstance()
{
    if(EnableValidationLayers_ && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested but not available!");
    }

    VkApplicationInfo app_info{};

    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = AppName_.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &app_info;

    //Vulkan needs extensions to interface with the GLFW window system.
    //GLFW has a built in function that returns the two parameters Vulkan needs:
    //  - the count of extensions       - the names of the extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if(EnableValidationLayers_)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers_.size());
        createInfo.ppEnabledLayerNames = ValidationLayers_.data();
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
    }
    

    //to retrive a list of supported extensions before creating an instance
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    //print the extensions
    for(VkExtensionProperties& extension : extensions)
    {
        std::cout << extension.extensionName << std::endl;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance_);

    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance");
    }

    return Instance_;
}

bool IVRInstanceManager::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(VkLayerProperties layerProperty : availableLayers)
    {
        std::cout << layerProperty.layerName << std::endl;
    }

    std::vector<bool> validation_layer_present;
    validation_layer_present.resize(ValidationLayers_.size());
    std::fill(validation_layer_present.begin(), validation_layer_present.end(), false);

    for(VkLayerProperties layerProperty : availableLayers)
    {
        for(int i = 0; i < validation_layer_present.size(); i++)
        {
            if(strcmp(layerProperty.layerName, ValidationLayers_[i]) == 0)
            {
                validation_layer_present[i] = true;
            }
        }
    }

    std::vector<bool>::iterator it = std::find(validation_layer_present.begin(), validation_layer_present.end(), false);

    if(it == validation_layer_present.end())
    {
        return true;
    }
    return false;
}

VkInstance IVRInstanceManager::GetInstance()
{
    return Instance_;
}
