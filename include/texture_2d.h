#pragma once

#include <vulkan/vulkan.h>

#include "texture.h"


class IVRTexture2D : public IVRTexture {
private:
	std::string TexturePath_;

public:
	IVRTexture2D(std::shared_ptr<IVRDeviceManager> device_manager, std::string texture_path);
	~IVRTexture2D() override; //virtual destructor
	
	void CreateTextureImage() override;
};
