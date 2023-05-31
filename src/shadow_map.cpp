#include "shadow_map.h"

IVRShadowMap::IVRShadowMap(std::shared_ptr<IVRDeviceManager> device_manager,std::shared_ptr<IVRLightManager> light_manager, VkExtent2D swapchain_extent, uint32_t swapchain_image_count) :
	DeviceManager_(device_manager), LightManager_(light_manager), SwapchainExtent_(swapchain_extent), SwapchainImageCount_(swapchain_image_count)
{
	SMVertexShaderPath_ = IVRPath::GetCrossPlatformPath({ "shaders", "shadow_map.vert.spv" });
	SMFragmentShaderPath_ = IVRPath::GetCrossPlatformPath({ "shaders", "shadow_map.frag.spv "});

	CreateLightMVPUniformBuffers();
	CreateDepthImage();
	CreateRenderpass();
	CreateFramebuffer();
	CreatePipeline();
}

void IVRShadowMap::CreateDepthImage()
{
	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		DepthImages_.push_back(std::make_shared<IVRDepthImage>(DeviceManager_, SwapchainExtent_));
	}
}

void IVRShadowMap::CreateFramebuffer()
{
	SMFramebuffers_.resize(SwapchainImageCount_);

	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		std::array<VkImageView, 1> attachments = {DepthImages_[i]->GetDepthImageView()};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = SMRenderpass_;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = SwapchainExtent_.width;
		framebuffer_info.height = SwapchainExtent_.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(DeviceManager_->GetLogicalDevice(), &framebuffer_info, nullptr, &SMFramebuffers_[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void IVRShadowMap::CreateLightMVPUniformBuffers()
{
	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		std::shared_ptr<IVRUBManager> ub_manager = std::make_shared<IVRUBManager>(DeviceManager_, static_cast<VkDeviceSize>(sizeof(ShadowMapLightMVPUBObj)));
		ShadowMapLightMVPUBObj ubo{};
		ub_manager->WriteToUniformBuffer(&ubo, static_cast<VkDeviceSize>(sizeof(ShadowMapLightMVPUBObj)));
		LightMVPUBObjs_.push_back(ubo);
		LightMVPUBManagers_.push_back(ub_manager);
	}
}


void IVRShadowMap::CreateRenderpass()
{
	IVRRenderpassConfig renderpass_config;
	renderpass_config.isUseDepthAttachment = true;
	VkAttachmentDescription depth_attachment_description = {};
	depth_attachment_description.format = DepthImages_[0]->FindDepthFormat();
	depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	renderpass_config.DepthAttachment = depth_attachment_description;

	//I think at this point it is better to create a new renderpass for the shadow map
	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 0;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpass_description{};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.flags = 0;
	subpass_description.inputAttachmentCount = 0;
	subpass_description.pInputAttachments = nullptr;
	subpass_description.colorAttachmentCount = 0;
	subpass_description.pColorAttachments = nullptr;
	subpass_description.pResolveAttachments = nullptr;
	subpass_description.pResolveAttachments = nullptr;
	subpass_description.pDepthStencilAttachment = &depth_attachment_ref;

	VkSubpassDependency subpass_dependency{};
	subpass_dependency.srcSubpass = 0;
	subpass_dependency.dstSubpass = VK_SUBPASS_EXTERNAL; //this means that commands of the next renderpass will wait for the commands of this subpass to finish
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; //this means that the fragment shader of the next renderpass will wait for fragment shader test of this subpass to finish
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpass_dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpass_dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderpass_create_info{};
	renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_create_info.attachmentCount = 1;
	renderpass_create_info.pAttachments = &depth_attachment_description;
	renderpass_create_info.subpassCount = 1;
	renderpass_create_info.pSubpasses = &subpass_description;
	renderpass_create_info.dependencyCount = 1;
	renderpass_create_info.pDependencies = &subpass_dependency;
	
	if (vkCreateRenderPass(DeviceManager_->GetLogicalDevice(), &renderpass_create_info, nullptr, &SMRenderpass_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create renderpass for shadow map");
	}
}

void IVRShadowMap::CreatePipeline()
{
	PipelineCreator_ = std::make_shared<IVRPipelineCreator>(DeviceManager_);
	IVRFixedFunctionPipelineConfig pipeline_config(SwapchainExtent_);

	std::shared_ptr<IVRDescriptorManager> descriptor_manager = std::make_shared<IVRDescriptorManager>(DeviceManager_);
	VkDescriptorSetLayout descriptor_set_layout = descriptor_manager->CreateDescriptorSetLayout(GetDescriptorSetInfo());

	SMPipelineLayout_ = PipelineCreator_->CreatePipelineLayout(descriptor_set_layout);
	SMPipeline_ = PipelineCreator_->CreatePipeline(SMRenderpass_, pipeline_config, SMPipelineLayout_, SMVertexShaderPath_, SMFragmentShaderPath_);
		
}

void IVRShadowMap::UpdateLightMVPUB(uint32_t swapchain_index, glm::mat4& model_mat)
{
	LightMVPUBObjs_[swapchain_index].Model = model_mat;

	LightMVPUBObjs_[swapchain_index].LightView = LightManager_->GetLight(0).GetLightView();

	float fov = 90.0f;
	float aspect_ratio = (float)2160 / 1440;
	float near_plane = 0.1f;
	float far_plane = 100.0f;
	LightMVPUBObjs_[swapchain_index].LightProjection = LightManager_->GetLight(0).GetLightProjection(fov, aspect_ratio, near_plane, far_plane);

	LightMVPUBManagers_[swapchain_index]->WriteToUniformBuffer(&LightMVPUBObjs_[swapchain_index], static_cast<VkDeviceSize>(sizeof(ShadowMapLightMVPUBObj)));
}

IVRDescriptorSetInfo IVRShadowMap::GetDescriptorSetInfo()
{
	IVRDescriptorSetInfo descriptor_set_info;

	VkDescriptorSetLayoutBinding ubo_layout_binding{};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.pImmutableSamplers = nullptr;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //this uniform buffer will contain MVP matrix from the light's point of view

	descriptor_set_info.DescriptorSetLayoutBindings.push_back(ubo_layout_binding);

	return descriptor_set_info;
}

VkPipeline IVRShadowMap::GetPipeline()
{
	return SMPipeline_;
}

VkPipelineLayout IVRShadowMap::GetPipelineLayout()
{
	return SMPipelineLayout_;
}

std::vector<std::shared_ptr<IVRDepthImage>> IVRShadowMap::GetDepthImages()
{
	return DepthImages_;
}

void IVRShadowMap::BeginRenderPass(VkCommandBuffer command_buffer, uint32_t swapchain_index)
{
	VkRenderPassBeginInfo renderpass_begin_info{};
	renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_begin_info.renderPass = SMRenderpass_;
	renderpass_begin_info.framebuffer = SMFramebuffers_[swapchain_index];
	renderpass_begin_info.renderArea.offset = { 0, 0 };
	renderpass_begin_info.renderArea.extent = { SwapchainExtent_.width, SwapchainExtent_.height };
	VkClearValue clear_value{};
	clear_value.depthStencil = { 1.0f, 0 };
	renderpass_begin_info.clearValueCount = 1;
	renderpass_begin_info.pClearValues = &clear_value;

	vkCmdBeginRenderPass(command_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

}

void IVRShadowMap::EndRenderPass(VkCommandBuffer command_buffer)
{
	vkCmdEndRenderPass(command_buffer);
}


