#pragma once

#include "texture.h"
#include "depth_image.h"

class IVRTextureDepth : public IVRTexture {
private:
	std::shared_ptr<IVRDepthImage> DepthImage_;

public:
	IVRTextureDepth(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRDepthImage> depth_image);
	~IVRTextureDepth();
	
	void CreateTextureImage() override;

};