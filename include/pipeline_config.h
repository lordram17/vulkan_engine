#pragma once

#include <vulkan/vulkan.h>

#include "model.h"

struct IVRFixedFunctionPipelineConfig {

	//Vertex input 
	VkPipelineVertexInputStateCreateInfo VertexInput{};
	VkVertexInputBindingDescription VertexBindingDescription;
	std::vector<VkVertexInputAttributeDescription> VertexAttributeDescriptions;
	
	//Input Assembly
	VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
	
	//Viewport and scissors
	bool isUseDynamicState = true;
	VkExtent2D Extent;
	VkViewport Viewport{};
	VkRect2D Scissor{};
	VkPipelineViewportStateCreateInfo ViewportState{};
	std::vector<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,	VK_DYNAMIC_STATE_SCISSOR };
	
	//Dynamic states
	VkPipelineDynamicStateCreateInfo DynamicState{};

	//Rasterizer
	VkPipelineRasterizationStateCreateInfo Rasterizer{};

	//Multisampling
	VkPipelineMultisampleStateCreateInfo Multisampling{};

	//Depth and stencil testing
	VkPipelineDepthStencilStateCreateInfo DepthStencil{};

	//Color blending
	VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo ColorBlending{};

	IVRFixedFunctionPipelineConfig(VkExtent2D extent)
	{
		isUseDynamicState = false;
		Extent = extent;

		SetDefaultValues();
	}

	IVRFixedFunctionPipelineConfig()
	{
		isUseDynamicState = true;
		SetDefaultValues();
	}

	void SetDefaultValues()
	{
		VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInput.pNext = nullptr;
		VertexInput.vertexBindingDescriptionCount = 1;
		VertexBindingDescription = Vertex::getBindingDescription();
		VertexInput.pVertexBindingDescriptions = &VertexBindingDescription;
		VertexAttributeDescriptions = Vertex::getAttributeDescription();
		VertexInput.vertexAttributeDescriptionCount = VertexAttributeDescriptions.size();
		VertexInput.pVertexAttributeDescriptions = VertexAttributeDescriptions.data();

		InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssembly.pNext = nullptr;
		InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssembly.primitiveRestartEnable = VK_FALSE;

		Viewport.x = 0.0f;
		Viewport.y = 0.0f;
		Viewport.width = (float)Extent.width;
		Viewport.height = (float)Extent.height;
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;

		Scissor.offset = { 0, 0 };
		Scissor.extent = Extent;

		ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportState.viewportCount = 1;
		ViewportState.pViewports = &Viewport;
		ViewportState.scissorCount = 1;
		ViewportState.pScissors = &Scissor;

		DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicState.pNext = nullptr;
		DynamicState.dynamicStateCount = 0; //initialized to 0. should be modified if dynamic states are used
		DynamicState.pDynamicStates = DynamicStates.data(); //using dynamic states by default

		Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		Rasterizer.pNext = nullptr;
		Rasterizer.depthClampEnable = VK_FALSE;
		Rasterizer.rasterizerDiscardEnable = VK_FALSE;
		Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		Rasterizer.lineWidth = 1.0f;
		Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		Rasterizer.depthBiasEnable = VK_FALSE;
		Rasterizer.depthBiasConstantFactor = 0.0f;
		Rasterizer.depthBiasClamp = 0.0f;
		Rasterizer.depthBiasSlopeFactor = 0.0f;

		Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		Multisampling.pNext = nullptr;
		Multisampling.sampleShadingEnable = VK_FALSE;
		Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		Multisampling.minSampleShading = 1.0f;
		Multisampling.pSampleMask = nullptr;
		Multisampling.alphaToCoverageEnable = VK_FALSE;
		Multisampling.alphaToOneEnable = VK_FALSE;

		DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencil.pNext = nullptr;
		DepthStencil.depthTestEnable = VK_TRUE;
		DepthStencil.depthWriteEnable = VK_TRUE;
		DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		DepthStencil.depthBoundsTestEnable = VK_FALSE;
		DepthStencil.minDepthBounds = 0.0f;
		DepthStencil.maxDepthBounds = 1.0f;
		DepthStencil.stencilTestEnable = VK_FALSE;
		DepthStencil.front = {};
		DepthStencil.back = {};

		ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachment.blendEnable = VK_FALSE;
		ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlending.pNext = nullptr;
		ColorBlending.logicOpEnable = VK_FALSE;
		ColorBlending.logicOp = VK_LOGIC_OP_COPY;
		ColorBlending.attachmentCount = 1;
		ColorBlending.pAttachments = &ColorBlendAttachment;
		ColorBlending.blendConstants[0] = 0.0f;
		ColorBlending.blendConstants[1] = 0.0f;
		ColorBlending.blendConstants[2] = 0.0f;
		ColorBlending.blendConstants[3] = 0.0f;
	}

};