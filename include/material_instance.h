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

#include "pipeline_config.h"


class IVRMaterialInstance {

private:
	std::string VertexShaderPath_;
	std::string FragmentShaderPath_;

	std::vector<std::string> TextureNames_;
	std::vector<std::shared_ptr<IVRTexture>> Textures_;

	std::vector<VkDescriptorSet> DescriptorSets_;
	VkDescriptorSetLayout DescriptorSetLayout_;
	IVRDescriptorSetInfo DescriptorSetInfo_;

	std::vector<std::shared_ptr<IVRUBManager>> MVPMatrixUBs_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;

	VkPipelineLayout PipelineLayout_;
	VkPipeline Pipeline_;

	MaterialPropertiesUBObj MaterialProperties_;

	uint32_t SwapchainImageCount_;

	//2D vector of lights (there are multiple lights for each swapchain image) (first index is swapchain image index, second index is light index)
	//even though light will generally remain constant, it can be changed every frame (for example, if the light is attached to a moving object)
	std::vector<std::vector<std::shared_ptr<IVRUBManager>>> LightUBs_; 
	uint32_t LightCount_;

	std::vector<std::shared_ptr<IVRUBManager>> MaterialPropertiesUBs_;

public:
	IVRMaterialInstance(std::shared_ptr<IVRDeviceManager> device_manager, std::string vertex_shader_name, std::string fragment_shader_name, 
				std::vector<std::string> texture_names, MaterialPropertiesUBObj properties, uint32_t swapchain_image_count, 
				std::vector<std::vector<std::shared_ptr<IVRUBManager>>>& light_ubos);

	//create the IVRDescriptorSetInfo that will be used to create the descriptor set layout
	void CreateDescriptorSetLayoutInfo();
	IVRDescriptorSetInfo GetDescriptorSetInfo();
	
	//Get the descriptor pool size for this material (the resources required by this material on the descriptor pool)
	std::vector<VkDescriptorPoolSize> GetDescriptorPoolSize();

	//IVRDescriptorManager creates the descriptor set layout, descriptor pool and descriptor sets. Here we just assign the descriptor set layout to the material
	void AssignDescriptorSetLayout(VkDescriptorSetLayout layout);
	VkDescriptorSetLayout GetDescriptorSetLayout();
	
	void AssignDescriptorSet(VkDescriptorSet descriptor_set);
	VkDescriptorSet GetDescriptorSet(uint32_t swapchain_image_index);

	//only the mvp matrix uniform buffer is written to the descriptor set (this is updated every frame)
	void WriteMVPMatrixToDescriptorSet(uint32_t swapchain_image_index);
	void WriteToDescriptorSet(uint32_t swapchain_image_index);

	void InitMVPMatrixUBs();
	std::shared_ptr<IVRUBManager> GetMVPMatrixUB(uint32_t swapchain_image_index);
	void SetLightsUBs(std::vector<std::shared_ptr<IVRUBManager>> light_ubs);
	void InitMaterialPropertiesUBs();

	void SetPipeline(VkPipeline pipeline);
	VkPipeline GetPipeline();
	void SetPipelineLayout(VkPipelineLayout pipeline_layout);
	VkPipelineLayout GetPipelineLayout();

	std::string GetVertexShaderPath();
	std::string GetFragmentShaderPath();

	void UpdatePipelineConfigBasedOnMaterialProperties(IVRFixedFunctionPipelineConfig& ff_pipeline_config);
};