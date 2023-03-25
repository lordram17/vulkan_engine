#include "device_setup.h"

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount); //contains type of operations that are support and the number of queues that can be created based on that family
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // the indices of the array queueFamilies are used by many other functions to reference queueFamilies

    int queueFamilyIndex = 0; 
    for(const auto& queueFamily : queueFamilies)
    {
        //need to check if a giving queueFamily is acceptable in terms of graphics support and presentation support
        //the queueFamilyIndex represents the index of the returned array from vkGetPhysicalDeviceQueueFamilyProperties
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = queueFamilyIndex;
            indices.isGraphicsFamilyIndexSet = true;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, surface, &presentSupport);

            if(presentSupport)
            {
                indices.isPresentFamilyIndexSet = true;
                indices.presentFamily = queueFamilyIndex;
            }
        }

        if(indices.isComplete())
        {
            break;
        }
        queueFamilyIndex++;
    }

    //logic to find graphics queue family
    return indices;
}

bool IVRDeviceCreator::IsDeviceSuitable_(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties deviceProperties; //basic device properties like name, type and supported vulkan version
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures; //support for optional features like texture compression, 64 bit float, multi viewport rendering
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // can add logic to check if device is suitable or choose from multiple devices based on a score alloted depending on their capabilities

    bool extensions_supported = CheckDeviceExtensionSupport_(device);

    QueueFamilyIndices indices = FindQueueFamilies(device, surface);

    return indices.isComplete() && extensions_supported;
}

bool IVRDeviceCreator::CheckDeviceExtensionSupport_(VkPhysicalDevice physical_device)
{
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions;

    for(std::string extension :DeviceExtensions_)
    {
        required_extensions.insert(extension);
    }

    for(const VkExtensionProperties& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

IVRDeviceCreator::IVRDeviceCreator()
{
}

IVRDeviceCreator::~IVRDeviceCreator()
{
    //vkDestroyDevice(LogicalDevice_, nullptr);
}

VkPhysicalDevice IVRDeviceCreator::PickPhysicalDevice(VkSurfaceKHR surface, VkInstance instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if(deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        if(IsDeviceSuitable_(device, surface))
        {
            PhysicalDevice_ = device;
            break;
        }
    }

    if(PhysicalDevice_ == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Found GPU/s but none of them are suitable");
    }

    PickedPhysicalDeviceQueueFamilyIndices_ = FindQueueFamilies(PhysicalDevice_, surface);

    return PhysicalDevice_;
}



VkDevice IVRDeviceCreator::CreateLogicalDevice(VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = PickedPhysicalDeviceQueueFamilyIndices_;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    for(uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f; //vulkan allows assigning of priorities to queues to influence the scheduling of command buffer execution using floating point between 0 and 1
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures; 
    //next, need to specify extensions and validation layers (device specific)
    //an example of a device specific extension is VK_KHR_swaphcain (there may be devices that lack this because they only support compute operations)
    createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions_.size());
    createInfo.ppEnabledExtensionNames = DeviceExtensions_.data();

    //IMPROTANT NOTE : older implementations of Vulkan support device specific validation layers. THIS IS NO LONGER THE CASE.
    //there the following 2 lines are only for backward compatibility. These fields are ignored by up to date Vulkan implementations.
    //createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    //createInfo.ppEnabledLayerNames = validationLayers.data();
    createInfo.enabledLayerCount = 0;


    if(vkCreateDevice(PhysicalDevice_, &createInfo, nullptr, &LogicalDevice_) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Logical Device");
    }

    vkGetDeviceQueue(LogicalDevice_, indices.graphicsFamily, 0, &GraphicsQueue_); 
    vkGetDeviceQueue(LogicalDevice_, indices.presentFamily, 0, &PresentQueue_);

    return LogicalDevice_;
}

VkQueue IVRDeviceCreator::GetGraphicsQueue()
{
    return GraphicsQueue_;
}

VkQueue IVRDeviceCreator::GetPresentQueue()
{
    return PresentQueue_;
}

QueueFamilyIndices IVRDeviceCreator::GetDeviceQueueFamilies()
{
    return PickedPhysicalDeviceQueueFamilyIndices_;
}
