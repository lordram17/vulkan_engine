#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>

#include "image_utils.h"
#include "device_setup.h"
#include "ivr_window.h"
#include "texture.h"

//Swapchain is essentially a queue of images waiting to be presented to the screen

struct SwapchainSupportDetails
{
    //there are 3 kinds of properties that need to be checked
    
    // 1. basic surface capabilities (min/max number of images in swapchain, min/max width and height of images)
    VkSurfaceCapabilitiesKHR capabilities;

    // 2. surface formals (pixel format, color space)
    std::vector<VkSurfaceFormatKHR> formats;

    // 3. available presentation modes
    std::vector<VkPresentModeKHR> presentModes;
};

class IVRSwapchainManager {

private:
    SwapchainSupportDetails QuerySwapchainSupport_(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    VkSwapchainKHR Swapchain_;
    std::vector<VkImage>  SwapchainImages_;
    std::vector<VkImageView> SwapchainImageViews_;
    VkFormat SwapchainImageFormat_;
    VkExtent2D SwapchainExtent_;
    std::vector<VkFramebuffer> SwapchainFramebuffers_;

    std::shared_ptr<IVRDeviceManager> DeviceManager_;
    std::shared_ptr<IVRWindow> Window_;

public:

    IVRSwapchainManager(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRWindow> window);
    ~IVRSwapchainManager(){};

    bool IsSwapchainAdequate(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void CreateSwapchain();
    void DestroySwapchain();
    void RetrieveSwapchainImages();

    //To use VkImage in the render pipeline (as render targets), we need to create a VkImageView object
    //Image view is a view into the image. it describes how to access the image and which part of the image to access
    // should it be treated as a 2D depth texture without any mipmapping levels
    void CreateImageViews();
    void DestroyImageViews();
    uint16_t GetImageViewCount();
    VkImageView GetImageViewByIndex(uint16_t index);

    VkFormat GetSwapchainImageFormat();
    VkExtent2D GetSwapchainExtent();
    VkSwapchainKHR GetSwapchain();

    void CreateFramebuffers(VkRenderPass renderPass, VkImageView depth_image_view);
    void DestroyFramebuffers();
    VkFramebuffer GetFramebuffer(uint32_t image_index);

};