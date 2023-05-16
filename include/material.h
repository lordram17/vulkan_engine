#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "texture.h"
#include "uniform_buffer_manager.h"
#include "descriptors.h"
#include "ivr_path.h"
#include "device_setup.h"


class IVRMaterial {

private:
	std::string VertexShaderPath_;
	std::string FragmentShaderPath_;

	std::vector<std::string> TextureNames_;
	std::vector<std::shared_ptr<IVRTexObj>> Textures_;

	VkDescriptorSet DescriptorSet_;
	VkDescriptorSetLayout DescriptorSetLayout_;
	IVRDescriptorSetInfo DescriptorSetInfo_;


	std::shared_ptr<IVRUBManager> MVPMatrixUB_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;

	VkPipelineLayout PipelineLayout_;
	VkPipeline Pipeline_;

public:
	IVRMaterial(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRUBManager> mvp_matrix_ub,
		std::string vertex_shader_name, std::string fragment_shader_name, std::vector<std::string> texture_names);

	//create the IVRDescriptorSetInfo that will be used to create the descriptor set layout
	void CreateDescriptorSetLayoutInfo();

	IVRDescriptorSetInfo GetDescriptorSetInfo();
	
	//Get the descriptor pool size for this material (the resources required by this material on the descriptor pool)
	std::vector<VkDescriptorPoolSize> GetDescriptorPoolSize();

	//IVRDescriptorManager creates the descriptor set layout, descriptor pool and descriptor sets. Here we just assign the descriptor set layout to the material
	void AssignDescriptorSetLayout(VkDescriptorSetLayout layout);
	VkDescriptorSetLayout GetDescriptorSetLayout();
	void AssignDescriptorSet(VkDescriptorSet descriptor_set);
	VkDescriptorSet GetDescriptorSet();
	void WriteToDescriptorSet();

	//only the mvp matrix uniform buffer is written to the descriptor set (this is updated every frame)
	void WriteMVPMatrixToDescriptorSet();

	void SetMVPMatrixUB(std::shared_ptr<IVRUBManager> mvp_matrix_ub);
	std::shared_ptr<IVRUBManager> GetMVPMatrixUB();

	void SetPipeline(VkPipeline pipeline);
	VkPipeline GetPipeline();
	void SetPipelineLayout(VkPipelineLayout pipeline_layout);
	VkPipelineLayout GetPipelineLayout();

	std::string GetVertexShaderPath();
	std::string GetFragmentShaderPath();
};