#pragma once

#include <vulkan/vulkan.h>

#include "device_setup.h"
#include "debug_logger_utils.h"

struct IVRSyncObjectsManager
{
private:

	std::shared_ptr<IVRDeviceManager> DeviceManager_;	

public:

	VkSemaphore ImageAvailableSemaphore;
	VkSemaphore RenderFinishedSemaphore;
	VkFence InFlightFence;

	IVRSyncObjectsManager(std::shared_ptr<IVRDeviceManager> device_manager);
	~IVRSyncObjectsManager();

	void CreateSemaphores();
	void DestroySemaphores();

	void CreateFences();
	void DestroyFences();
};
