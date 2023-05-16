#include "sync_objects_manager.h"

IVRSyncObjectsManager::IVRSyncObjectsManager(std::shared_ptr<IVRDeviceManager> device_manager) :
	DeviceManager_(device_manager)
{
	CreateSemaphores();
	CreateFences();
}

IVRSyncObjectsManager::~IVRSyncObjectsManager()
{
	DestroySemaphores();
	DestroyFences();
}

void IVRSyncObjectsManager::CreateSemaphores()
{
	IVR_LOG_INFO("Creating semaphores...");
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(DeviceManager_->GetLogicalDevice(), &semaphore_create_info, nullptr, &ImageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(DeviceManager_->GetLogicalDevice(), &semaphore_create_info, nullptr, &RenderFinishedSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create semaphores!");
	}
}

void IVRSyncObjectsManager::DestroySemaphores()
{
	IVR_LOG_INFO("Destroying semaphores...");
	vkDestroySemaphore(DeviceManager_->GetLogicalDevice(), RenderFinishedSemaphore, nullptr);
	vkDestroySemaphore(DeviceManager_->GetLogicalDevice(), ImageAvailableSemaphore, nullptr);
}

void IVRSyncObjectsManager::CreateFences()
{
	IVR_LOG_INFO("Creating fences...");
	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(DeviceManager_->GetLogicalDevice(), &fence_create_info, nullptr, &InFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create fences!");
	}
}

void IVRSyncObjectsManager::DestroyFences()
{
	IVR_LOG_INFO("Destroying fences...");
	vkDestroyFence(DeviceManager_->GetLogicalDevice(), InFlightFence, nullptr);
}


