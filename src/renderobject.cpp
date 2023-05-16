#include "renderobject.h"

IVRRenderObject::IVRRenderObject(std::shared_ptr<IVRModel> model, std::shared_ptr<IVRMaterial> material, std::shared_ptr<IVRCamera> camera)
: Model_(model), Material_(material), Camera_(camera)
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

std::shared_ptr<IVRUBManager> IVRRenderObject::GetMVPMatrixUB()
{
    return Material_->GetMVPMatrixUB();
}

void IVRRenderObject::UpdateMVPMatrixUB()
{
    MVPMatrixObj.model = Model_->GetTransform().GetModelMatrix();
    MVPMatrixObj.view = Camera_->GetViewMatrix();
    MVPMatrixObj.proj = Camera_->GetProjectionMatrix();

    Material_->GetMVPMatrixUB()->WriteToUniformBuffer(&MVPMatrixObj, static_cast<VkDeviceSize>(sizeof(MVPUniformBufferObject)));
    
}

