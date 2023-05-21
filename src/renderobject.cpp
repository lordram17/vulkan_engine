#include "renderobject.h"

IVRRenderObject::IVRRenderObject(std::shared_ptr<IVRModel> model, std::shared_ptr<IVRMaterial> material, std::shared_ptr<IVRCamera> camera, uint32_t swapchain_image_count)
: Model_(model), Material_(material), Camera_(camera), SwapchainImageCount_(swapchain_image_count)
{
}

std::shared_ptr<IVRModel> IVRRenderObject::GetModel()
{
    return Model_;
}

std::shared_ptr<IVRMaterial> IVRRenderObject::GetMaterial()
{
    return Material_;
}

std::shared_ptr<IVRUBManager> IVRRenderObject::GetMVPMatrixUB(uint32_t swapchain_image_index)
{
    return Material_->GetMVPMatrixUB(swapchain_image_index);
}

void IVRRenderObject::UpdateMVPMatrixUB(uint32_t swapchain_image_index)
{
    MVPMatrixObj.model = Model_->GetTransform().GetModelMatrix();
    MVPMatrixObj.view = Camera_->GetViewMatrix();
    MVPMatrixObj.proj = Camera_->GetProjectionMatrix();

    Material_->GetMVPMatrixUB(swapchain_image_index)->WriteToUniformBuffer(&MVPMatrixObj, static_cast<VkDeviceSize>(sizeof(MVPUniformBufferObject)));
}

