#include "renderobject.h"

IVRRenderObject::IVRRenderObject(std::shared_ptr<IVRModel> model, std::shared_ptr<IVRMaterialInstance> material, std::shared_ptr<IVRCamera> camera, uint32_t swapchain_image_count)
: Model_(model), Material_(material), Camera_(camera), SwapchainImageCount_(swapchain_image_count)
{
}

std::shared_ptr<IVRModel> IVRRenderObject::GetModel()
{
    return Model_;
}

std::shared_ptr<IVRMaterialInstance> IVRRenderObject::GetMaterialInstance()
{
    return Material_;
}

std::shared_ptr<IVRUBManager> IVRRenderObject::GetMVPMatrixUB(uint32_t swapchain_image_index)
{
    return Material_->GetMVPMatrixUB(swapchain_image_index);
}

void IVRRenderObject::UpdateMVPMatrixUB(uint32_t swapchain_image_index)
{
    MVPMatrixObj.Model = Model_->GetTransform().GetModelMatrix();
    MVPMatrixObj.View = Camera_->GetViewMatrix();
    MVPMatrixObj.Proj = Camera_->GetProjectionMatrix();

    Material_->GetMVPMatrixUB(swapchain_image_index)->WriteToUniformBuffer(&MVPMatrixObj, static_cast<VkDeviceSize>(sizeof(MVPUBObj)));
}

void IVRRenderObject::UpdateLightMVPUB(uint32_t swapchain_image_index, glm::mat4 light_view, glm::mat4 light_proj)
{
    LightMVPUBObj.Model = Model_->GetTransform().GetModelMatrix();
	LightMVPUBObj.LightView = light_view;
	LightMVPUBObj.LightProjection = light_proj;
	
    ShadowmapMaterial_->GetLightMVPUB(swapchain_image_index)->WriteToUniformBuffer(&LightMVPUBObj, static_cast<VkDeviceSize>(sizeof(ShadowMapLightMVPUBObj)));
}

void IVRRenderObject::AssignLightMVPUBToMaterialInstance()
{
    for (uint32_t i = 0; i < SwapchainImageCount_; i++)
    {
        Material_->AssignLightMVPUniformBuffers(ShadowmapMaterial_->GetLightMVPUB(i));
    }
}

void IVRRenderObject::AssignShadowmapMaterial(std::shared_ptr<IVRShadowmapMaterial> shadowmap_material)
{
    ShadowmapMaterial_ = shadowmap_material;
}

std::shared_ptr<IVRShadowmapMaterial> IVRRenderObject::GetShadowmapMaterial()
{
    return ShadowmapMaterial_;
}

