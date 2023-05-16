#pragma once

#include <vulkan/vulkan.h>

#include "model.h"
#include "material.h"
#include "camera.h"
#include "ub_structs.h"
#include "uniform_buffer_manager.h"

//serves as the link between the model and the material
class IVRRenderObject {

private:

	std::shared_ptr<IVRModel> Model_;
	std::shared_ptr<IVRMaterial> Material_;
	std::shared_ptr<IVRCamera> Camera_;
	MVPUniformBufferObject MVPMatrixObj;

public:
	
	IVRRenderObject(std::shared_ptr<IVRModel> model, std::shared_ptr<IVRMaterial> material, std::shared_ptr<IVRCamera> camera);

	std::shared_ptr<IVRModel> GetModel();
	std::shared_ptr<IVRMaterial> GetMaterial();
	std::shared_ptr<IVRUBManager> GetMVPMatrixUB();

	void UpdateMVPMatrixUB();

};