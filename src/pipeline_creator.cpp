#pragma once

#include "pipeline_creator.h"
#include "simple_file_reader.h"

IVRPipelineCreator::IVRPipelineCreator(std::shared_ptr<IVRDeviceManager> device_manager) :
	DeviceManager_(device_manager)
{
}

VkPipeline IVRPipelineCreator::CreatePipeline(VkRenderPass render_pass, IVRFixedFunctionPipelineConfig ff_pipeline_config, VkPipelineLayout pipeline_layout,
												std::string vertex_shader_path, std::string fragment_shader_path)
{
	VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
	vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_stage_info.module = CreateShaderModule(vertex_shader_path);
	vertex_shader_stage_info.pName = "main";
	
	VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
	fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_stage_info.module = CreateShaderModule(fragment_shader_path);
	fragment_shader_stage_info.pName = "main";
	
	VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_stage_info, fragment_shader_stage_info };
	
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	
	//shaders
	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;

	//fixed function
	pipeline_info.pVertexInputState = &ff_pipeline_config.VertexInput;
	pipeline_info.pInputAssemblyState = &ff_pipeline_config.InputAssembly;
	pipeline_info.pViewportState = &ff_pipeline_config.ViewportState;
	pipeline_info.pRasterizationState = &ff_pipeline_config.Rasterizer;
	pipeline_info.pMultisampleState = &ff_pipeline_config.Multisampling;
	pipeline_info.pDepthStencilState = &ff_pipeline_config.DepthStencil;
	pipeline_info.pColorBlendState = &ff_pipeline_config.ColorBlending;

	pipeline_info.pViewportState = &ff_pipeline_config.ViewportState;
	pipeline_info.pDynamicState = &ff_pipeline_config.DynamicState;

	/*if (ff_pipeline_config.isUseDynamicState)
	{
		pipeline_info.pViewportState = nullptr;
		pipeline_info.pDynamicState = &ff_pipeline_config.DynamicState;
	}
	else
	{
		pipeline_info.pViewportState = &ff_pipeline_config.ViewportState;
		pipeline_info.pDynamicState = nullptr;
	}*/
	
	//renderpass
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;

	pipeline_info.layout = pipeline_layout;

	VkPipeline pipeline;

	if (vkCreateGraphicsPipelines(DeviceManager_->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	return pipeline;
}

VkPipelineLayout IVRPipelineCreator::CreatePipelineLayout(VkDescriptorSetLayout descriptor_set_layout)
{
	//pipeline layout
	VkPipelineLayout pipeline_layout;
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(DeviceManager_->GetLogicalDevice(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	return pipeline_layout;
}

VkShaderModule IVRPipelineCreator::CreateShaderModule(std::string shader_path)
{
	std::vector<char> bytecode = SimpleFileReader::ReadFile(shader_path.c_str());

	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = bytecode.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(bytecode.data()); //bytecode.data() returns char* but pCode expects uint32_t*
	create_info.pNext = nullptr;
	create_info.flags = 0;

	VkShaderModule shader_module;
	if (vkCreateShaderModule(DeviceManager_->GetLogicalDevice(), &create_info, nullptr, &shader_module) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}
	
	return shader_module;
}
