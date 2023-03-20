#include "swapchain_manager.h"

SwapchainSupportDetails SwapchainManager::QuerySwapchainSupport_(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails support_details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support_details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    
    if(format_count != 0)
    {
        support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, support_details.formats.data());
    }
    
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

    if(present_mode_count != 0)
    {
        support_details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, support_details.presentModes.data());
    }    
    

    return support_details;
}


bool SwapchainManager::IsSwapchainAdequate(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails swapchain_support_details = QuerySwapchainSupport_(physical_device, surface);
    bool swapchain_adequate = !swapchain_support_details.formats.empty() && !swapchain_support_details.presentModes.empty();
    
    return swapchain_adequate;
}

VkSurfaceFormatKHR SwapchainManager::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    //VkSurfaceFormatKHR contains a format and colorSpace member
    // format specifies color channels and type
    // colorSpace indicates if SRGB color space is supported or not using VK_COLOR_SPACE_SRGB_NONLINEAR_KHR flag
    //SRGB results in more accurate perceived colors

    for(const auto& available_format : available_formats)
    {
        if(available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR SwapchainManager::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes)
{
    /**
     *  the different present modes:
     * 
     * 1. VK_PRESENT_MODE_IMMEDIATE_KHR : images submitted by application are transferred to the screen right away (results in tearing)
     * 
     * 2. VK_PRESENT_MODE_FIFO_KHR : swachain is a queue where the display takes the image from the front of the queue. 
     *                              when display is refreshed, program inserts the rendered images to the back of the queue.
     *                              if queue is full the program has to wait
     *                              most similar to verical sync found in games. the moment the display is refreshed is called 'vertical blank'
     * 
     * 3. VK_PRESENT_MODE_FIFO_RELAXED_KHR : (variation of 2nd) if the queue was empty at the last 'vertical blank' (if the application was late) then as soon
     *                                      as the image arrives, it is transferred to the screen. This may result in tearing.
     * 
     * 4. VK_PRESENT_MODE_MAILBOX_KHR : (variation of 2nd) if the queue is full, instead of blocking newly rendered images, 
     *                                  new images will replace older ones still in the queue
     *                                  (good when energy usage is not a concern, so not the best for mobile devices)
     */

    for(const auto& available_present_mode : available_present_modes)
    {
        if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_mode;
        }
    }

    //FIFO is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainManager::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    /** 
     * Swap extent is the resolution of the swap chain images and it is almost always exactly equal to the resolution of the window
     * that we are drawing to (in pixels).
     * 
     * Vulkan tells us to match the resolution of the window by setting the width and height in the CurrentExtent member.
     * 
     * GLFW uses 2 units when measuring sizes : pixel and screen coordinates
     * for high DPI displays, this might not be the same
     * the WIDTH and HEIGHT specified when creating the GLFW window was in screen coordinates
     * 
     * for vulkan we need pixel sizes, so we need to use glfwGetFramebufferSize
     * 
    */

    return capabilities.currentExtent;
}

void SwapchainManager::CreateSwapchain(VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkSurfaceKHR surface, QueueFamilyIndices queue_families)
{
    SwapchainSupportDetails support_details = QuerySwapchainSupport_(physical_device, surface);

    VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(support_details.formats);
    VkPresentModeKHR present_mode = ChooseSwapPresentMode(support_details.presentModes);
    VkExtent2D extent = ChooseSwapExtent(support_details.capabilities);
    
    SwapchainExtent_ = extent;
    SwapchainImageFormat_ = surface_format.format;

    uint32_t image_count = support_details.capabilities.minImageCount + 1;

    if(support_details.capabilities.maxImageCount > 0 && image_count > support_details.capabilities.maxImageCount)
    {
        image_count = support_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = image_count;
    createInfo.imageFormat = surface_format.format;
    createInfo.imageColorSpace = surface_format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // the number of layers each image consists of. Always 1 unless stereoscopic 3D application
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //what kind of operations we will use the images in the swapchain for
    //VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : this is when we are rendering directly to this image
    //VK_IMAGE_USAGE_TRANSFER_DST_BIT : when we render images to a separate image first (maybe to perform postprocessing) and then transfer it to the swapchain image


    //There are two possible modes for image sharing
    // 1. VK_SHARING_MODE_EXCLUSIVE : image is owned by 1 queue family at a time and ownsership must be explicitly transferred 
    //    before using it in another queue family
    // 2. VK_SHARING_MODE_CONCURRENT : images can be used across multiple queue family without explicit ownership transfer

    // The logic being used below is that if the graphics queue and the present queue are different, then we need to allow
    // them to share the images (this is just for ease of use right now)
    // If the graphics and present queue are the same then we can use the exclusive mode since only one queue family will
    // be using the images
    if(queue_families.graphicsFamily != queue_families.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        uint32_t queue_family_indices_array[] = {queue_families.graphicsFamily, queue_families.presentFamily};
        createInfo.pQueueFamilyIndices = queue_family_indices_array;
    }   
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; //OPTIONAL
        createInfo.pQueueFamilyIndices = nullptr; //OPTIONAL
    }
    
    //can specify a transform to be applied to swapchain images if it is supported (checked using supportTransforms in capabilities)
    //this could be 90 degree rotation or horizontal flip
    //setting this to current transform means that we donot want to apply any transform
    createInfo.preTransform = support_details.capabilities.currentTransform;

    //specifies if alpha channel should be used for blending with other windows in the window system.
    //almost always this should be ignored (hence opaque bit specified)
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = present_mode;

    //meant for performance : choose clipping for best performance
    //setting this to true means that we dont care about color of pixels that are obscured (for example by the window in front)
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logical_device, &createInfo, nullptr, &Swapchain_) != VK_SUCCESS)
    {
        throw std::runtime_error("Familed to create swapchain");
    }

}

void SwapchainManager::DestroySwapchain(VkDevice logical_device)
{
    vkDestroySwapchainKHR(logical_device, Swapchain_, nullptr);
}

void SwapchainManager::RetrieveSwapchainImages(VkDevice logical_device)
{
    uint32_t image_count;
    vkGetSwapchainImagesKHR(logical_device, Swapchain_, &image_count, nullptr);
    SwapchainImages_.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device, Swapchain_, &image_count, SwapchainImages_.data());
}

//this function is not yet called anywhere
void SwapchainManager::CreateImageViews(VkDevice logical_device)
{
    SwapchainImageViews_.resize(SwapchainImages_.size());

    for(size_t i = 0; i < SwapchainImageViews_.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = SwapchainImages_[i];

        //viewtype specifies how the image data should be interpreted
        //allows you to treat images as 1D, 2D, 3D textures and cubemaps
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = SwapchainImageFormat_;

        //components field allows to swizzle the color channel around (swizzling = rearranging color channels)
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

        //subresourceRange specifies the image's purpose and which part of the image should be accessed
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1; //only 1 mipmap level
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1; //only 1 layer [more than 1 for stereographic 3D applications]

        if(vkCreateImageView(logical_device, &createInfo, nullptr, &SwapchainImageViews_[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view");
        }
    }
}

void SwapchainManager::DestroyImageViews(VkDevice logical_device)
{
    for(VkImageView& image_view : SwapchainImageViews_)
    {
        vkDestroyImageView(logical_device, image_view, nullptr);
    }
}

VkFormat SwapchainManager::GetSwapchainImageFormat()
{
    return SwapchainImageFormat_;
}

VkExtent2D SwapchainManager::GetSwapchainExtent()
{
    return SwapchainExtent_;
}
