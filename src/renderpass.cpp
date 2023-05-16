#include "renderpass.h"

IVRRenderpass::IVRRenderpass(IVRRenderpassConfig renderpass_config, std::shared_ptr<IVRDeviceManager> device_manager) :
	RenderpassConfig_(renderpass_config), DeviceManager_(device_manager)
{
	CreateRenderpass();
}

IVRRenderpass::~IVRRenderpass()
{
	vkDestroyRenderPass(DeviceManager_->GetLogicalDevice(), Renderpass_, nullptr);
}

VkRenderPass IVRRenderpass::GetRenderpass()
{
	return Renderpass_;
}


void IVRRenderpass::CreateRenderpass()
{
	//Creating subpass description
	SubpassDescription_ = {};
	//creating a vector of VkAttachmentReference for the color attachments
	std::vector<VkAttachmentReference> color_attachment_descriptions;

	for (int i = 0; i < RenderpassConfig_.ColorAttachments.size(); i++) {
		VkAttachmentReference color_attachment_reference{};
		color_attachment_reference.attachment = i;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment_descriptions.push_back(color_attachment_reference);
	}

	VkAttachmentReference depth_attachment_reference{};
	depth_attachment_reference.attachment = RenderpassConfig_.ColorAttachments.size();
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//creating the subpass description
	SubpassDescription_.flags = 0;
	SubpassDescription_.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription_.colorAttachmentCount = RenderpassConfig_.ColorAttachments.size();
	SubpassDescription_.pColorAttachments = color_attachment_descriptions.data();
	SubpassDescription_.pDepthStencilAttachment = &depth_attachment_reference;

	//creating the subpass dependency
	SubpassDependency_ = {};
	SubpassDependency_.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency_.dstSubpass = 0;
	SubpassDependency_.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency_.srcAccessMask = 0;
	SubpassDependency_.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	uint32_t attachment_count = static_cast<uint32_t>(RenderpassConfig_.ColorAttachments.size() + RenderpassConfig_.isUseDepthAttachment);
	std::vector<VkAttachmentDescription> attachment_descriptions;
	for (VkAttachmentDescription& attachment_description : RenderpassConfig_.ColorAttachments) {
		attachment_descriptions.push_back(attachment_description);
	}
	if (RenderpassConfig_.isUseDepthAttachment) {
		attachment_descriptions.push_back(RenderpassConfig_.DepthAttachment);
	}

	VkRenderPassCreateInfo renderpass_create_info{};
	renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_create_info.attachmentCount = attachment_count;
	renderpass_create_info.pAttachments = attachment_descriptions.data();
	renderpass_create_info.subpassCount = 1;
	renderpass_create_info.pSubpasses = &SubpassDescription_;
	renderpass_create_info.dependencyCount = 1;
	renderpass_create_info.pDependencies = &SubpassDependency_;

	if (vkCreateRenderPass(DeviceManager_->GetLogicalDevice(), &renderpass_create_info, nullptr, &Renderpass_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void IVRRenderpass::BeginRenderPass(VkCommandBuffer command_buffer, VkFramebuffer framebuffer, VkExtent2D extent)
{
	VkRenderPassBeginInfo renderpass_begin_info{};
	renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_begin_info.renderPass = Renderpass_;
	renderpass_begin_info.framebuffer = framebuffer;
	renderpass_begin_info.renderArea.offset = { 0, 0 };
	renderpass_begin_info.renderArea.extent = extent;
	std::vector<VkClearValue> clear_values(RenderpassConfig_.ColorAttachments.size() + RenderpassConfig_.isUseDepthAttachment);
	
	for (int i = 0; i < RenderpassConfig_.ColorAttachments.size(); i++) {
		clear_values[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	}
	
	if (RenderpassConfig_.isUseDepthAttachment) {
		clear_values[RenderpassConfig_.ColorAttachments.size()].depthStencil = { 1.0f, 0 };
	}
	
	renderpass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	renderpass_begin_info.pClearValues = clear_values.data();
	
	vkCmdBeginRenderPass(command_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void IVRRenderpass::EndRenderPass(VkCommandBuffer command_buffer)
{
	vkCmdEndRenderPass(command_buffer);
}
