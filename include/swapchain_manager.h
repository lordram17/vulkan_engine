#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "device_setup.h"
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

public:

    IVRSwapchainManager(){};
    ~IVRSwapchainManager(){};

    bool IsSwapchainAdequate(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void CreateSwapchain(VkDevice logical_device, VkPhysicalDevice physical_device, 
                        VkSurfaceKHR surface, QueueFamilyIndices queue_families);
    
    void DestroySwapchain(VkDevice logical_device);

    void RetrieveSwapchainImages(VkDevice logical_device);

    //To use VkImage in the render pipeline (as render targets), we need to create a VkImageView object
    //Image view is a view into the image. it describes how to access the image and which part of the image to access
    // should it be treated as a 2D depth texture without any mipmapping levels
    void CreateImageViews(VkDevice logical_device);
    void DestroyImageViews(VkDevice logical_device);
    uint16_t GetImageViewCount();

    VkFormat GetSwapchainImageFormat();
    VkExtent2D GetSwapchainExtent();
    VkSwapchainKHR GetSwapchain();

    void CreateFramebuffers(VkRenderPass renderPass, VkDevice logical_device);
    void DestroyFramebuffers(VkDevice logical_device);
    VkFramebuffer GetFramebuffer(uint32_t image_index);

};