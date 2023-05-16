#include "framebuffer_manager.h"

IVRFramebufferManager::IVRFramebufferManager(std::shared_ptr<IVRDeviceManager> device_manager, VkRenderPass renderpass, 
											std::shared_ptr<IVRSwapchainManager> swapchain_manager, std::shared_ptr<IVRDepthImage> depth_image) :
								DeviceManager_(device_manager), Renderpass_(renderpass), SwapchainManager_(swapchain_manager), DepthImage_(depth_image)
{
	CreateFramebuffers();
}

IVRFramebufferManager::~IVRFramebufferManager()
{
	DestroyFramebuffers();
}

void IVRFramebufferManager::CreateFramebuffers()
{
	IVR_LOG_INFO("Creating Framebuffers...");

	//create the framebuffers
	Framebuffers_.resize(SwapchainManager_->GetImageViewCount());
	
	for (size_t i = 0; i < SwapchainManager_->GetImageViewCount(); i++)
	{
		std::array<VkImageView, 2> attachments = { SwapchainManager_->GetImageViewByIndex(i), DepthImage_->GetDepthImageView()};
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = Renderpass_;
		framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = SwapchainManager_->GetSwapchainExtent().width;
		framebuffer_info.height = SwapchainManager_->GetSwapchainExtent().height;
		framebuffer_info.layers = 1;
		
		if (vkCreateFramebuffer(DeviceManager_->GetLogicalDevice(), &framebuffer_info, nullptr, &Framebuffers_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void IVRFramebufferManager::DestroyFramebuffers()
{	IVR_LOG_INFO("Destroying Framebuffers...");
	for (auto framebuffer : Framebuffers_)
	{
		vkDestroyFramebuffer(DeviceManager_->GetLogicalDevice(), framebuffer, nullptr);
	}
}

