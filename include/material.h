#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "texture.h"
#include "texture_2d.h"
#include "texture_cube.h"
#include "uniform_buffer_manager.h"
#include "descriptors.h"
#include "ivr_path.h"
#include "light.h"
#include "device_setup.h"

#include "pipeline_config.h"

struct MaterialProperties {
	bool IsCubemap = false;
};

class IVRMaterial {

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

	MaterialProperties MaterialProperties_;

	uint32_t SwapchainImageCount_;

public:
	IVRMaterial(std::shared_ptr<IVRDeviceManager> device_manager, std::string vertex_shader_name, std::string fragment_shader_name, 
				std::vector<std::string> texture_names, MaterialProperties properties, uint32_t swapchain_image_count);

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
	void WriteToDescriptorSet(uint32_t swapchain_image_index);

	//only the mvp matrix uniform buffer is written to the descriptor set (this is updated every frame)
	void WriteMVPMatrixToDescriptorSet(uint32_t swapchain_image_index);

	void SetMVPMatrixUB(std::shared_ptr<IVRUBManager> mvp_matrix_ub);
	std::shared_ptr<IVRUBManager> GetMVPMatrixUB(uint32_t swapchain_image_index);

	void SetPipeline(VkPipeline pipeline);
	VkPipeline GetPipeline();
	void SetPipelineLayout(VkPipelineLayout pipeline_layout);
	VkPipelineLayout GetPipelineLayout();

	std::string GetVertexShaderPath();
	std::string GetFragmentShaderPath();

	void UpdatePipelineConfigBasedOnMaterialProperties(IVRFixedFunctionPipelineConfig& ff_pipeline_config);
};