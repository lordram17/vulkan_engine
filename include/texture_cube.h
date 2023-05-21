#pragma once

#include "texture.h"


class IVRTextureCube : public IVRTexture {
private:

	IVRTextureCube(const IVRTextureCube&) = delete;
	std::vector<std::string> CubemapPaths_;



public:
	IVRTextureCube(std::shared_ptr<IVRDeviceManager> device_manager, std::string cubemap_folder_path);
	~IVRTextureCube() override; //virtual destructor

	void CreateTextureImage() override;
};