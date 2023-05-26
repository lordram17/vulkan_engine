#pragma once

#include "descriptors.h"
#include "pipeline_config.h"

class IVRBaseMaterial {

private:
	std::string Name_;
	std::string VertexShaderPath_;
	std::string FragmentShaderPath_;
	std::string DefaultTexture_;

	VkDescriptorSetLayout DescriptorSetLayout_;
	IVRDescriptorSetInfo DescriptorSetInfo_;
	VkPipelineLayout PipelineLayout_;
	VkPipeline Pipeline_;

	uint32_t LightCount_;
	uint32_t TextureCount_;
	uint32_t SwapchainImageCount_;

	bool IsCubemap = false;

public:
	IVRBaseMaterial(std::string name, std::string vertex_shader_path, std::string fragment_shader_path, std::string default_texture,
		uint32_t light_count, uint32_t texture_count, uint32_t swapchain_image_count, bool is_cubemap);

	std::string GetVertexShaderPath();
	std::string  GetFragmentShaderPath();
	std::string GetDefaultTexture();

	void CreateDescriptorSetLayoutInfo();
	IVRDescriptorSetInfo GetDescriptorSetInfo();

	void SetDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout);
	VkDescriptorSetLayout GetDescriptorSetLayout();

	std::vector<VkDescriptorPoolSize> GetDescriptorPoolSize();

	void SetPipeline(VkPipeline pipeline);
	VkPipeline GetPipeline();
	void  SetPipelineLayout(VkPipelineLayout pipeline_layout);
	VkPipelineLayout  GetPipelineLayout();

	void UpdatePipelineConfigBasedOnMaterialProperties(IVRFixedFunctionPipelineConfig& ff_pipeline_config);
};