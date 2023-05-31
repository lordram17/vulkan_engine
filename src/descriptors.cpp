#include "descriptors.h"

IVRDescriptorManager::IVRDescriptorManager(std::shared_ptr<IVRDeviceManager> device_manager) :
	DeviceManager_(device_manager)
{
}

VkDescriptorSetLayout IVRDescriptorManager::CreateDescriptorSetLayout(IVRDescriptorSetInfo& descriptor_set_info)
{
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
	descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(descriptor_set_info.DescriptorSetLayoutBindings.size());
	descriptor_set_layout_create_info.pBindings = descriptor_set_info.DescriptorSetLayoutBindings.data();

	VkDescriptorSetLayout descriptor_set_layout; 
	if (vkCreateDescriptorSetLayout(DeviceManager_->GetLogicalDevice(), &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}

	return descriptor_set_layout;
}

void IVRDescriptorManager::CreateDescriptorPool(std::vector<VkDescriptorPoolSize>& descriptor_pool_sizes, uint32_t max_sets)
{
	VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
	descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
	descriptor_pool_create_info.maxSets = max_sets;
	
	if (vkCreateDescriptorPool(DeviceManager_->GetLogicalDevice(), &descriptor_pool_create_info, nullptr, &DescriptorPool_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}


VkDescriptorSet IVRDescriptorManager::CreateDescriptorSet(VkDescriptorSetLayout descriptor_set_layout)
{
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info{}; 
	descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
	descriptor_set_allocate_info.descriptorPool = DescriptorPool_; 
	descriptor_set_allocate_info.descriptorSetCount = 1; 
	descriptor_set_allocate_info.pSetLayouts = &descriptor_set_layout; 

	VkDescriptorSet descriptor_set; 
	if (vkAllocateDescriptorSets(DeviceManager_->GetLogicalDevice(), &descriptor_set_allocate_info, &descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set!");
	}

	return descriptor_set;
}

