#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "device_setup.h"
#include "pipeline_config.h"

class IVRPipelineCreator
{
private:
	std::shared_ptr<IVRDeviceManager> DeviceManager_;

public:
	IVRPipelineCreator(std::shared_ptr<IVRDeviceManager> device_manager);
	~IVRPipelineCreator() {};

	VkPipeline CreatePipeline(VkRenderPass render_pass, IVRFixedFunctionPipelineConfig ff_pipeline_config, VkPipelineLayout pipeline_layout,
								std::string vertex_shader_path, std::string fragment_shader_path);

	VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layouts);

	VkShaderModule CreateShaderModule(std::string shader_path);

};