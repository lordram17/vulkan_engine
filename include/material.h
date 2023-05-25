#pragma once


#include "descriptors.h"

class Material {

private:
	std::string VertexShaderPath_;
	std::string FragmentShaderPath_;

	VkDescriptorSetLayout DescriptorSetLayout_;
	IVRDescriptorSetInfo DescriptorSetInfo_;
	VkPipelineLayout PipelineLayout_;
	VkPipeline Pipeline_;

public:

};