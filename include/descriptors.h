#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <memory>

#include "device_setup.h"


struct IVRDescriptorSetInfo {
	//contains information for creating a descriptor set	
	uint32_t DescriptorSetID;
	
	//index in the descriptor set layout bindings vector should be the same as binding id in the descriptor set
	std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;
};


//This class can be used to create a descriptor pool, descriptor set layouts and descriptor sets which are then passed to the Pipeline to be used for rendering
class IVRDescriptorManager {
	//There are 5 pieces to descriptor sets
	//1. Descriptor Set Layout Bindings
	//2. Descriptor Set Layout
	//3. Descriptor Pool
	//4. Descriptor Sets
	//5. Update Descriptor Sets (write to descriptor sets)


	//1. Descriptor Set Layout Bindings
	//This is a list of all the descriptors that will be used in the shader
	//Lets make it so that the class requires an input of these bindings

private:
	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	VkDescriptorPool DescriptorPool_;

public:
	IVRDescriptorManager(std::shared_ptr<IVRDeviceManager> device_manager);

	VkDescriptorSetLayout CreateDescriptorSetLayout(IVRDescriptorSetInfo& descriptor_set_info);
	void CreateDescriptorPool(std::vector<VkDescriptorPoolSize>& descriptor_pool_sizes, uint32_t max_sets);
	VkDescriptorSet CreateDescriptorSet(VkDescriptorSetLayout descriptor_set_layout);

};