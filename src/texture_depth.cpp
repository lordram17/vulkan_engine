#include "texture_depth.h"

IVRTextureDepth::IVRTextureDepth(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRDepthImage> depth_image) :
	IVRTexture(device_manager), DepthImage_(depth_image)
{
	TextureImageView_ = DepthImage_->GetDepthImageView();
	ImageViewType_ = VK_IMAGE_VIEW_TYPE_2D;
	SamplerAddressMode_ = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	TextureFormat_ = DepthImage_->FindDepthFormat();
	CreateTextureSampler();
}

IVRTextureDepth::~IVRTextureDepth()
{
}

void IVRTextureDepth::CreateTextureImage()
{
	//nothing since depth image is already created
}


