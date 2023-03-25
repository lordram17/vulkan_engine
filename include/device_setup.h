#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <stdexcept>

struct QueueFamilyIndices 
{
    uint32_t graphicsFamily = 0;
    bool isGraphicsFamilyIndexSet = false;

    uint32_t presentFamily = 0;
    bool isPresentFamilyIndexSet = false;

    bool isComplete()
    {
        return isGraphicsFamilyIndexSet && isPresentFamilyIndexSet;
    }
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

/**
 * @brief respoinsible for creating the logical and physical devices
 * 
 */
class IVRDeviceCreator {

private:
    const std::vector<const char*> DeviceExtensions_ = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    }; //the above macro simply gets converted to "VK_KHR_swapchain"
    //the reason for using the macro is so that compiler can catch error in case there is a mispelling

    VkPhysicalDevice PhysicalDevice_;
    VkDevice LogicalDevice_; 

    VkQueue GraphicsQueue_;
    VkQueue PresentQueue_;

    QueueFamilyIndices PickedPhysicalDeviceQueueFamilyIndices_;

    bool IsDeviceSuitable_(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    
    bool CheckDeviceExtensionSupport_(VkPhysicalDevice physical_device);

public:
    IVRDeviceCreator();
    ~IVRDeviceCreator();

    VkPhysicalDevice PickPhysicalDevice(VkSurfaceKHR surface, VkInstance instance);
    VkDevice CreateLogicalDevice(VkSurfaceKHR surface);

    VkQueue GetGraphicsQueue();
    VkQueue GetPresentQueue();

    QueueFamilyIndices GetDeviceQueueFamilies();

};