#pragma once

#include <vulkan/vulkan.h>

#include "renderpass.h"
#include "device_setup.h"
#include "depth_image.h"
#include "framebuffer_manager.h"
#include "pipeline_creator.h"
#include "descriptors.h"
#include "uniform_buffer_manager.h"
#include "ub_structs.h"
#include "light_manager.h"


class IVRShadowMap {
private:
	std::string SMVertexShaderPath_;
	std::string SMFragmentShaderPath_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	std::shared_ptr<IVRLightManager> LightManager_;
	std::vector<std::shared_ptr<IVRDepthImage>> DepthImages_;
	std::shared_ptr<IVRPipelineCreator> PipelineCreator_;
	
	uint32_t SwapchainImageCount_;
	VkExtent2D SwapchainExtent_;
	
	VkRenderPass SMRenderpass_;
	std::vector<VkFramebuffer> SMFramebuffers_;
	VkPipelineLayout SMPipelineLayout_;
	VkPipeline SMPipeline_;

	std::vector<ShadowMapLightMVPUBObj> LightMVPUBObjs_;
	std::vector<std::shared_ptr<IVRUBManager>> LightMVPUBManagers_;

public:

	IVRShadowMap(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRLightManager> light_manager, VkExtent2D swapchain_extent, uint32_t swapchain_image_count);

	void CreateDepthImage();
	void CreateRenderpass();
	void CreateFramebuffer();
	void CreateLightMVPUniformBuffers();
	void CreatePipeline();

	void UpdateLightMVPUB(uint32_t swapchain_index, glm::mat4& model_mat);

	void BeginRenderPass(VkCommandBuffer command_buffer, uint32_t swapchain_index);
	void EndRenderPass(VkCommandBuffer command_buffer);

	IVRDescriptorSetInfo GetDescriptorSetInfo();
	VkPipeline GetPipeline();
	VkPipelineLayout GetPipelineLayout();

	std::vector<std::shared_ptr<IVRDepthImage>> GetDepthImages();
};


