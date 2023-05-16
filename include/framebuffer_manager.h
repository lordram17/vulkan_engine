#pragma once

#include <vulkan/vulkan.h>

#include "device_setup.h"
#include "swapchain_manager.h"
#include "depth_image.h"
#include "debug_logger_utils.h"

class IVRFramebufferManager
{
private:

	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	std::shared_ptr<IVRSwapchainManager> SwapchainManager_;
	std::shared_ptr<IVRDepthImage> DepthImage_;
	VkRenderPass Renderpass_;

	std::vector<VkFramebuffer> Framebuffers_;

public:

	IVRFramebufferManager(std::shared_ptr<IVRDeviceManager> device_manager, VkRenderPass renderpass,
							std::shared_ptr<IVRSwapchainManager> swapchain_manager,	std::shared_ptr<IVRDepthImage> depth_image);
	~IVRFramebufferManager();

	void CreateFramebuffers();
	void DestroyFramebuffers();

	VkFramebuffer GetFramebuffer(int index) { return Framebuffers_[index]; }

};