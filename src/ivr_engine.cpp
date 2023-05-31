#include "ivr_engine.h"


IVREngine::IVREngine()
{
	InitEngine();
}

void IVREngine::InitEngine()
{
	//setup logger
	IVRDebugLogger::InitLogger();

	IVR_LOG_INFO("Initializing Engine");

	IVR_LOG_INFO("Initializing GLFW and Creating Window...");
	Window_ = std::make_shared<IVRWindow>(2160, 1440);
	Window_->InitWindow();
	
	IVR_LOG_INFO("Creating Vulkan instance...");
	//setup instance
	InstanceManager_ = std::make_shared<IVRInstanceManager>();
	//InstanceManager_->EnableValidationLayersForInstanceCreation();
	InstanceManager_->SetAppName("i Vulkan Renderer");
	InstanceManager_->CreateVulkanInstance();
	
	IVR_LOG_INFO("Creating Vulkan Surface...");
	Window_->CreateWindowSurface(InstanceManager_->GetInstance());

	IVR_LOG_INFO("Creating Devices and Device Manager...");
	DeviceManager_ = std::make_shared<IVRDeviceManager>();
	DeviceManager_->PickPhysicalDevice(Window_->GetVulkanSurface(), InstanceManager_->GetInstance());
	DeviceManager_->CreateLogicalDevice(Window_->GetVulkanSurface());

	IVR_LOG_INFO("Creating the Swapchain...");
	SwapchainManager_ = std::make_shared<IVRSwapchainManager>(DeviceManager_, Window_);
	SwapchainManager_->CreateSwapchain(); //create the swapchain (chooses format, present mode and extent) (chooses the number of images in the swapchain)
	SwapchainManager_->RetrieveSwapchainImages(); //retrieve the swapchain images and populate a vector of VkImage objects in the swapchain manager
	SwapchainManager_->CreateImageViews(); //create the image views for the swapchain images and populate a vector of VkImageView objects in the swapchain manager

	IVR_LOG_INFO("Creating the Depth Image...");
	DepthImage_ = std::make_shared<IVRDepthImage>(DeviceManager_, SwapchainManager_->GetSwapchainExtent());

	IVR_LOG_INFO("Creating the renderpass");
	CreateRenderpass();

	IVR_LOG_INFO("Creating the Framebuffers");
	FramebufferManager_ = std::make_shared<IVRFramebufferManager>(DeviceManager_, Renderpass_->GetRenderpass(), SwapchainManager_, DepthImage_);
	SyncObjectsManager_ = std::make_shared<IVRSyncObjectsManager>(DeviceManager_);

	IVR_LOG_INFO("Creating the Command Buffer");
	CBManager_ = std::make_shared<IVRCBManager>(DeviceManager_);
}

void IVREngine::PostWorldInit()
{
	IVR_LOG_INFO("Creating the shadow mapper");
	ShadowMap_ = std::make_shared<IVRShadowMap>(DeviceManager_, World_->GetLightManager(), SwapchainManager_->GetSwapchainExtent(), SwapchainManager_->GetImageViewCount());
	World_->InitShadowMapMaterials(ShadowMap_->GetDescriptorSetInfo());
	World_->AssignShadowMapDepthTextures(ShadowMap_->GetDepthImages());
	World_->PostShadowMapperInit();

	World_->SetCameraAspectRatio(SwapchainManager_->GetSwapchainExtent().width / (float)SwapchainManager_->GetSwapchainExtent().height);
	PipelineCreator_ = std::make_shared<IVRPipelineCreator>(DeviceManager_);
	IVR_LOG_INFO("Creating Pipelines...");
	CreatePipelines();
}

void IVREngine::CreateRenderpass()
{
	VkAttachmentDescription color_attachment_description{};
	color_attachment_description.format = SwapchainManager_->GetSwapchainImageFormat();
	color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment_description{};
	depth_attachment_description.format = DepthImage_->FindDepthFormat();
	depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	IVRRenderpassConfig renderpass_config;

	renderpass_config.ColorAttachments.push_back(color_attachment_description);
	renderpass_config.isUseDepthAttachment = true;
	renderpass_config.DepthAttachment = depth_attachment_description;

	Renderpass_ = std::make_shared<IVRRenderpass>(renderpass_config, DeviceManager_);
}

void IVREngine::CreatePipelines()
{
	//create pipelines
	for (std::shared_ptr<IVRBaseMaterial>& base_material : World_->GetBaseMaterials())
	{

		IVRFixedFunctionPipelineConfig pipeline_config(SwapchainManager_->GetSwapchainExtent());
		base_material->UpdatePipelineConfigBasedOnMaterialProperties(pipeline_config);

		VkPipelineLayout pipeline_layout = PipelineCreator_->CreatePipelineLayout(base_material->GetDescriptorSetLayout());
		VkPipeline pipeline = PipelineCreator_->CreatePipeline(Renderpass_->GetRenderpass(), pipeline_config,
			pipeline_layout, base_material->GetVertexShaderPath(), base_material->GetFragmentShaderPath());

		base_material->SetPipeline(pipeline);
		base_material->SetPipelineLayout(pipeline_layout);
	}
}


void IVREngine::DrawFrame()
{
	vkResetCommandBuffer(CBManager_->GetCommandBuffer(), 0);
	
	CBManager_->StartCommandBuffer();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)SwapchainManager_->GetSwapchainExtent().width;
	viewport.height = (float)SwapchainManager_->GetSwapchainExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(CBManager_->GetCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = SwapchainManager_->GetSwapchainExtent();
	vkCmdSetScissor(CBManager_->GetCommandBuffer(), 0, 1, &scissor);

	//shadow map rendering
	ShadowMap_->BeginRenderPass(CBManager_->GetCommandBuffer(), CurrentSwapchainImageIndex_);
	vkCmdBindPipeline(CBManager_->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, ShadowMap_->GetPipeline());

	for (std::shared_ptr<IVRRenderObject> render_object : World_->GetRenderObjects())
	{
		VkDescriptorSet sm_descriptor_set[] = { render_object->GetShadowmapMaterial()->GetDescriptorSet(CurrentSwapchainImageIndex_)};
		vkCmdBindDescriptorSets(CBManager_->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, ShadowMap_->GetPipelineLayout(), 0, 1, sm_descriptor_set, 0, nullptr);

		VkBuffer vertex_buffers[] = { render_object->GetModel()->GetVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CBManager_->GetCommandBuffer(), 0, 1, vertex_buffers, offsets);
		vkCmdBindIndexBuffer(CBManager_->GetCommandBuffer(), render_object->GetModel()->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(CBManager_->GetCommandBuffer(), render_object->GetModel()->Indices.size(), 1, 0, 0, 0);
	}
	ShadowMap_->EndRenderPass(CBManager_->GetCommandBuffer());

	Renderpass_->BeginRenderPass(CBManager_->GetCommandBuffer(), FramebufferManager_->GetFramebuffer(CurrentSwapchainImageIndex_), SwapchainManager_->GetSwapchainExtent());

	for (std::unordered_map<std::shared_ptr<IVRBaseMaterial>, std::vector<std::shared_ptr<IVRRenderObject>>>::iterator iter = World_->GetBaseMaterialRenderObjectMap().begin();
		iter != World_->GetBaseMaterialRenderObjectMap().end(); ++iter)
	{
		std::shared_ptr<IVRBaseMaterial> base_material = iter->first;
		std::vector<std::shared_ptr<IVRRenderObject>> render_objects = iter->second;

		vkCmdBindPipeline(CBManager_->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, base_material->GetPipeline());

		for (std::shared_ptr<IVRRenderObject> render_object : render_objects) 
		{
			VkBuffer vertex_buffers[] = { render_object->GetModel()->GetVertexBuffer() }; 
			VkDeviceSize offsets[] = { 0 };
			
			vkCmdBindVertexBuffers(CBManager_->GetCommandBuffer(), 0, 1, vertex_buffers, offsets);
			vkCmdBindIndexBuffer(CBManager_->GetCommandBuffer(), render_object->GetModel()->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			
			VkDescriptorSet descriptor_sets[] = { render_object->GetMaterialInstance()->GetDescriptorSet(CurrentSwapchainImageIndex_)};
			vkCmdBindDescriptorSets(CBManager_->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, base_material->GetPipelineLayout(), 0, 1, descriptor_sets, 0, nullptr);
			
			vkCmdDrawIndexed(CBManager_->GetCommandBuffer(), render_object->GetModel()->Indices.size(), 1, 0, 0, 0);
		}
	}

	Renderpass_->EndRenderPass(CBManager_->GetCommandBuffer());
	CBManager_->EndCommandBuffer();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { SyncObjectsManager_->ImageAvailableSemaphore };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	VkCommandBuffer command_buffers[] = { CBManager_->GetCommandBuffer() };
	submit_info.pCommandBuffers = command_buffers;

	VkSemaphore signal_semaphores[] = { SyncObjectsManager_->RenderFinishedSemaphore };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	if (vkQueueSubmit(DeviceManager_->GetGraphicsQueue(), 1, &submit_info, SyncObjectsManager_->InFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//present
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapchains[] = { SwapchainManager_->GetSwapchain() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &CurrentSwapchainImageIndex_;
	present_info.pResults = nullptr;

	vkQueuePresentKHR(DeviceManager_->GetPresentQueue(), &present_info);
}

uint32_t IVREngine::QueryForSwapchainIndex()
{
	//get next image from swapchain
	vkWaitForFences(DeviceManager_->GetLogicalDevice(), 1, &SyncObjectsManager_->InFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(DeviceManager_->GetLogicalDevice(), 1, &SyncObjectsManager_->InFlightFence);

	vkAcquireNextImageKHR(DeviceManager_->GetLogicalDevice(), SwapchainManager_->GetSwapchain(), UINT64_MAX,
		SyncObjectsManager_->ImageAvailableSemaphore, VK_NULL_HANDLE, &CurrentSwapchainImageIndex_);
	
	return CurrentSwapchainImageIndex_;
}


