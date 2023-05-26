#pragma once

#include "json.hpp"

#include "renderobject.h"
#include "device_setup.h"
#include "light_manager.h"

class IVRWorldLoader {
private:
	std::vector<IVRLight> Lights_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	std::shared_ptr<IVRLightManager> LightManager_;
	std::shared_ptr<IVRCamera> Camera_;
	uint32_t SwapchainImageCount_;

	std::unordered_map<std::string, std::shared_ptr<IVRBaseMaterial>> NameBaseMaterialMap_;

public:
	IVRWorldLoader(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRLightManager> light_manager, std::shared_ptr<IVRCamera> camera, uint32_t swapchain_image_count);
	~IVRWorldLoader();

	std::vector<std::shared_ptr<IVRBaseMaterial>> LoadBaseMaterialsFromJson();
	std::vector<std::shared_ptr<IVRRenderObject>>  LoadRenderObjectsFromJson();
	std::vector<IVRLight>&& LoadLightsFromJson();
};
