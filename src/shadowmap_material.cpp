#include "shadowmap_material.h"

IVRShadowmapMaterial::IVRShadowmapMaterial(std::shared_ptr<IVRDeviceManager> device_manager, IVRDescriptorSetInfo descriptor_set_info, uint32_t swapchain_image_count) :
	DeviceManager_(device_manager), SMDescriptorSetInfo_(descriptor_set_info),	SwapchainImageCount_(swapchain_image_count)
{
	InitLightMVPUBs();
}

IVRDescriptorSetInfo& IVRShadowmapMaterial::GetDescriptorSetInfo()
{
	return SMDescriptorSetInfo_;
}

std::vector<VkDescriptorPoolSize> IVRShadowmapMaterial::GetDescriptorPoolSize()
{
	std::vector<VkDescriptorPoolSize> descriptor_pool_size;

	for (VkDescriptorSetLayoutBinding& binding : SMDescriptorSetInfo_.DescriptorSetLayoutBindings)
	{
		VkDescriptorPoolSize pool_size{};
		pool_size.type = binding.descriptorType;
		pool_size.descriptorCount = binding.descriptorCount * SwapchainImageCount_;
		
		descriptor_pool_size.push_back(pool_size);
	}

	return descriptor_pool_size;
}

void IVRShadowmapMaterial::AssignDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout)
{
	SMDescriptorSetLayout_ = descriptor_set_layout;
}

VkDescriptorSetLayout IVRShadowmapMaterial::GetDescriptorSetLayout()
{
	return SMDescriptorSetLayout_;
}

void IVRShadowmapMaterial::AssignDescriptorSet(VkDescriptorSet descriptor_set)
{
	SMDescriptorSets_.push_back(descriptor_set);
}

void IVRShadowmapMaterial::WriteToDescriptorSet(uint32_t swapchain_index)
{
	std::vector<VkWriteDescriptorSet> write_descriptor_sets;
	
	for (VkDescriptorSetLayoutBinding& binding : SMDescriptorSetInfo_.DescriptorSetLayoutBindings)
	{
		VkDescriptorBufferInfo buffer_info;
		buffer_info.buffer = LightMVPUBManagers_[swapchain_index]->GetBuffer();
		buffer_info.offset = 0;
		buffer_info.range = sizeof(ShadowMapLightMVPUBObj);

		VkWriteDescriptorSet write_descriptor_set{};
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set.dstSet = SMDescriptorSets_[swapchain_index];
		write_descriptor_set.dstBinding = binding.binding;
		write_descriptor_set.dstArrayElement = 0;
		write_descriptor_set.descriptorType = binding.descriptorType;
		write_descriptor_set.descriptorCount = binding.descriptorCount;
		write_descriptor_set.pBufferInfo = &buffer_info;
		write_descriptor_set.pImageInfo = nullptr;
		write_descriptor_set.pTexelBufferView = nullptr;
		
		write_descriptor_sets.push_back(write_descriptor_set);
	}
	
	vkUpdateDescriptorSets(DeviceManager_->GetLogicalDevice(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}

VkDescriptorSet IVRShadowmapMaterial::GetDescriptorSet(uint32_t swapchain_index)
{
	return SMDescriptorSets_[swapchain_index];
}

void IVRShadowmapMaterial::InitLightMVPUBs()
{
	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		std::shared_ptr<IVRUBManager> light_mvp_ub = std::make_shared<IVRUBManager>(DeviceManager_, sizeof(ShadowMapLightMVPUBObj));
		LightMVPUBManagers_.push_back(light_mvp_ub);
	}
}

std::shared_ptr<IVRUBManager> IVRShadowmapMaterial::GetLightMVPUB(uint32_t swapchain_image_index)
{
	return LightMVPUBManagers_[swapchain_image_index];
}



