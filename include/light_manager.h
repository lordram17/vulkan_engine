#pragma once

#include "ub_structs.h"
#include "debug_logger_utils.h"
#include "uniform_buffer_manager.h"
#include "light.h"
#include "device_setup.h"

class IVRLightManager
{

private:
	//2D array of light ub managers, first dimension is swapchain image index, second dimension is light index
	std::vector<std::vector<std::shared_ptr<IVRUBManager>>> LightUBManagers_;
	std::vector<std::vector<LightUBObj>> LightUBOs_;
	std::vector<IVRLight> Lights_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	uint32_t SwapchainImageCount_;

public:

	IVRLightManager(std::shared_ptr<IVRDeviceManager> device_manager, uint32_t swapchain_image_count);

	void SetupLights(std::vector<IVRLight>&& lights);
	void TransformLightsByViewMatrix(glm::mat4 view, uint32_t swapchain_index);

	uint32_t GetLightCount();
	void UpdateLightByIndex(uint32_t light_index);
	IVRLight& GetLightByIndex(uint32_t light_index);
	std::shared_ptr<IVRUBManager> GetLightUBManagerByIndex(uint32_t light_index, uint32_t swapchain_index);
	std::vector<std::vector<std::shared_ptr<IVRUBManager>>>& GetAllLightUBs();

	IVRLight& GetLight(uint32_t index);

};