#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>

#include "device_setup.h"
#include "buffer_utils.h"
#include "image_utils.h"
#include "singlecommand_utils.h"

class IVRTexObj {

private:

    //disable copy constructor and assignment operator
    IVRTexObj(const IVRTexObj&) = delete;

    std::string TexturePath_;

    std::shared_ptr<IVRDeviceManager> DeviceManager_;

    VkImage TextureImage_;
    VkDeviceMemory TextureImageMemory_;
    VkImageView TextureImageView_;
    VkSampler TextureSampler_;

public:
    IVRTexObj(std::shared_ptr<IVRDeviceManager> device_manager, std::string texture_path);

    void CreateTextureImage();    

    void CreateTextureImageView();
    VkImageView GetTextureImageView();

    void CreateTextureSampler();
    VkSampler GetTextureSampler();

    void CleanUp();

};

