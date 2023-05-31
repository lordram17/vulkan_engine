#pragma once

#include <vulkan/vulkan.h>

#include "model.h"
#include "material_instance.h"
#include "shadowmap_material.h"
#include "camera.h"
#include "ub_structs.h"
#include "uniform_buffer_manager.h"
#include "pipeline_config.h"

//serves as the link between the model and the material
class IVRRenderObject {

private:

	std::shared_ptr<IVRModel> Model_;
	std::shared_ptr<IVRMaterialInstance> Material_;
	std::shared_ptr<IVRShadowmapMaterial> ShadowmapMaterial_;
	std::shared_ptr<IVRCamera> Camera_;
	MVPUBObj MVPMatrixObj;
	ShadowMapLightMVPUBObj LightMVPUBObj;
	uint32_t SwapchainImageCount_;

public:
	
	IVRRenderObject(std::shared_ptr<IVRModel> model, std::shared_ptr<IVRMaterialInstance> material, std::shared_ptr<IVRCamera> camera, uint32_t swapchain_image_count);

	std::shared_ptr<IVRModel> GetModel();
	std::shared_ptr<IVRMaterialInstance> GetMaterialInstance();
	std::shared_ptr<IVRUBManager> GetMVPMatrixUB(uint32_t swapchain_image_index);

	void UpdateMVPMatrixUB(uint32_t swapchain_image_index);
	void UpdateLightMVPUB(uint32_t swapchain_image_index, glm::mat4 light_view, glm::mat4 light_proj);
	void AssignLightMVPUBToMaterialInstance();

	void AssignShadowmapMaterial(std::shared_ptr<IVRShadowmapMaterial> shadowmap_material);
	std::shared_ptr<IVRShadowmapMaterial> GetShadowmapMaterial();

};