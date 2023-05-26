#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "descriptors.h"
#include "device_setup.h"
#include "renderobject.h"
#include "camera.h"
#include "light_manager.h"
#include "world_loader.h"
#include "debug_logger_utils.h"
#include "material_instance.h"

//there should be only one world. Render objects can be grouped together into a scene (for now doing it directly)
class IVRWorld {

private:
	std::shared_ptr <IVRDescriptorManager> DescriptorManager_; //this class creates it own descriptor manager
	std::shared_ptr<IVRDeviceManager> DeviceManager_;

	std::vector<std::shared_ptr<IVRRenderObject>> RenderObjects_;
	std::vector<std::shared_ptr<IVRBaseMaterial>> BaseMaterials_;
	//when drawing frame, bind the pipeline once and then draw all objects with the same pipeline (they may/will have different descriptor sets)
	std::unordered_map<std::shared_ptr<IVRBaseMaterial>, std::vector<std::shared_ptr<IVRRenderObject>>> BaseMaterialRenderObjectMap_;

	std::shared_ptr<IVRCamera> Camera_;
	
	std::shared_ptr<IVRLightManager> LightManager_;
	
	uint32_t SwapchainImageCount_;

public:

	IVRWorld(std::shared_ptr<IVRDeviceManager> device_manager, uint32_t swapchain_image_count);

	void SetupCamera();
	void SetCameraAspectRatio(float aspect_ratio);

	void CreateDescriptorSetLayoutsForBaseMaterials();
	std::vector<VkDescriptorPoolSize> CountPoolSizes();
	void CreateDescriptorSets();
	void WriteDescriptorSets();

	void Init();
	void Update(float dt, uint32_t swapchain_index);

	std::vector<std::shared_ptr<IVRRenderObject>>& GetRenderObjects();
	std::vector<std::shared_ptr<IVRBaseMaterial>>& GetBaseMaterials();
	void OrganizeRenderObjectsByBaseMaterial();
	std::unordered_map<std::shared_ptr<IVRBaseMaterial>, std::vector<std::shared_ptr<IVRRenderObject>>>& GetBaseMaterialRenderObjectMap();

};