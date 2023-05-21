#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>


#include "stb_image.h"

#include "device_setup.h"
#include "buffer_utils.h"
#include "image_utils.h"
#include "singlecommand_utils.h"
#include "debug_logger_utils.h"


class IVRTexture {

private:

    //disable copy constructor and assignment operator
    IVRTexture(const IVRTexture&) = delete;

protected:
    std::shared_ptr<IVRDeviceManager> DeviceManager_;

    VkImage TextureImage_;
    VkDeviceMemory TextureImageMemory_;
    VkImageView TextureImageView_;
    VkSampler TextureSampler_;
    
    VkImageViewType ImageViewType_;
    VkSamplerAddressMode SamplerAddressMode_;
    VkFormat TextureFormat_;
    uint32_t LayerCount_ = 1;

    void CleanUp();

public:
    IVRTexture(std::shared_ptr<IVRDeviceManager> device_manager);
    virtual ~IVRTexture();

    virtual void CreateTextureImage() = 0;    
    virtual void CreateTextureImageView();
    virtual void CreateTextureSampler();
    
    void InitTexture();
    VkImage GetTextureImage();
    VkImageView GetTextureImageView();
    VkSampler GetTextureSampler();

};

