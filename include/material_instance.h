#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "texture.h"
#include "texture_2d.h"
#include "texture_cube.h"
#include "uniform_buffer_manager.h"
#include "descriptors.h"
#include "ivr_path.h"
#include "ub_structs.h"
#include "device_setup.h"
#include "material.h"


class IVRMaterialInstance {

private:
	std::shared_ptr<IVRBaseMaterial> BaseMaterial_;

	std::vector<std::string> TextureNames_;
	std::vector<std::shared_ptr<IVRTexture>> Textures_;

	std::vector<VkDescriptorSet> DescriptorSets_;

	std::vector<std::shared_ptr<IVRUBManager>> MVPMatrixUBs_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;

	MaterialPropertiesUBObj MaterialProperties_;

	uint32_t SwapchainImageCount_;

	//2D vector of lights (there are multiple lights for each swapchain image) (first index is swapchain image index, second index is light index)
	//even though light will generally remain constant, it can be changed every frame (for example, if the light is attached to a moving object)
	std::vector<std::vector<std::shared_ptr<IVRUBManager>>> LightUBs_; 
	uint32_t LightCount_;

	std::vector<std::shared_ptr<IVRUBManager>> MaterialPropertiesUBs_;

public:
	IVRMaterialInstance(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRBaseMaterial> base_material,
				std::vector<std::string> texture_names, MaterialPropertiesUBObj properties, uint32_t swapchain_image_count, 
				std::vector<std::vector<std::shared_ptr<IVRUBManager>>>& light_ubos);
	
	void AssignDescriptorSet(VkDescriptorSet descriptor_set);
	VkDescriptorSet GetDescriptorSet(uint32_t swapchain_image_index);

	//only the mvp matrix uniform buffer is written to the descriptor set (this is updated every frame)
	void WriteMVPMatrixToDescriptorSet(uint32_t swapchain_image_index);
	void WriteToDescriptorSet(uint32_t swapchain_image_index);

	void InitMVPMatrixUBs();
	std::shared_ptr<IVRUBManager> GetMVPMatrixUB(uint32_t swapchain_image_index);
	void SetLightsUBs(std::vector<std::shared_ptr<IVRUBManager>> light_ubs);
	void InitMaterialPropertiesUBs();

	std::shared_ptr<IVRBaseMaterial> GetBaseMaterial();
};