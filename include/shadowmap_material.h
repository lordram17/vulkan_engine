#pragma once

#include "descriptors.h"
#include "ub_structs.h"
#include "uniform_buffer_manager.h"

class IVRShadowmapMaterial {

private:
	IVRDescriptorSetInfo SMDescriptorSetInfo_;
	std::vector<VkDescriptorSet> SMDescriptorSets_;
	VkDescriptorSetLayout SMDescriptorSetLayout_;
	VkDescriptorPool SMDescriptorPool_;
	std::vector<std::shared_ptr<IVRUBManager>> LightMVPUBManagers_;

	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	uint32_t SwapchainImageCount_;

public:

	IVRShadowmapMaterial(std::shared_ptr<IVRDeviceManager> device_manager, IVRDescriptorSetInfo descriptor_set_info, uint32_t swapchain_image_count);
	
	IVRDescriptorSetInfo& GetDescriptorSetInfo();

	
	std::vector<VkDescriptorPoolSize> GetDescriptorPoolSize();
	void AssignDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout);
	VkDescriptorSetLayout GetDescriptorSetLayout();
	void AssignDescriptorSet(VkDescriptorSet descriptor_set);
	void WriteToDescriptorSet(uint32_t swapchain_index);
	VkDescriptorSet GetDescriptorSet(uint32_t swapchain_index);

	void InitLightMVPUBs();
	std::shared_ptr<IVRUBManager> GetLightMVPUB(uint32_t swapchain_image_index);

};