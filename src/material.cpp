#include "material.h"

IVRMaterial::IVRMaterial(std::shared_ptr<IVRDeviceManager> device_manager, 
	std::shared_ptr<IVRUBManager> mvp_matrix_ub,
	std::string vertex_shader_name, std::string fragment_shader_name, 
	std::vector<std::string> texture_names) :
	DeviceManager_(device_manager), MVPMatrixUB_(mvp_matrix_ub)
{
	VertexShaderPath_ = IVRPath::GetCrossPlatformPath({"shaders", vertex_shader_name });
	FragmentShaderPath_ = IVRPath::GetCrossPlatformPath({ "shaders", fragment_shader_name });

	for (std::string texture_name : texture_names)
	{
		std::string texture_path = IVRPath::GetCrossPlatformPath({ "texture_files", texture_name });
		std::shared_ptr<IVRTexObj> texture_object = std::make_shared<IVRTexObj>(DeviceManager_, texture_path);
		Textures_.push_back(texture_object);
	}

	CreateDescriptorSetLayoutInfo();
}

void IVRMaterial::CreateDescriptorSetLayoutInfo()
{
	DescriptorSetInfo_ = {};
	
	//assign the mvp matrix uniform buffer always to the binding 0
	VkDescriptorSetLayoutBinding mvp_matrix_binding{};
	mvp_matrix_binding.binding = 0;
	mvp_matrix_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_binding.descriptorCount = 1;
	mvp_matrix_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvp_matrix_binding.pImmutableSamplers = nullptr; // Optional

	DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(mvp_matrix_binding);
	
	uint32_t binding_count = 1; //the mvp matrix uniform buffer is always at binding 0, so we start at 1 for the next bindings

	//assign the texture samplers to the next bindings
	for (std::shared_ptr<IVRTexObj> texture_object : Textures_)
	{
		VkDescriptorSetLayoutBinding texture_binding{};
		texture_binding.binding = binding_count;
		texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texture_binding.descriptorCount = 1;
		texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		texture_binding.pImmutableSamplers = nullptr; // Optional

		DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(texture_binding);
		binding_count++;
	}
}

IVRDescriptorSetInfo IVRMaterial::GetDescriptorSetInfo()
{
	return DescriptorSetInfo_;
}

std::vector<VkDescriptorPoolSize> IVRMaterial::GetDescriptorPoolSize()
{
	std::vector<VkDescriptorPoolSize> descriptor_pool_size;
	
	//the mvp matrix size is always 1
	VkDescriptorPoolSize mvp_matrix_pool_size{};
	mvp_matrix_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_pool_size.descriptorCount = 1;
	descriptor_pool_size.push_back(mvp_matrix_pool_size);
	
	//the texture samplers may be more than one dependending on the number of textures
	VkDescriptorPoolSize texture_pool_size{};
	texture_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texture_pool_size.descriptorCount = Textures_.size();
	descriptor_pool_size.push_back(texture_pool_size);

	return descriptor_pool_size;
}

void IVRMaterial::AssignDescriptorSetLayout(VkDescriptorSetLayout layout)
{
	DescriptorSetLayout_ = layout;
}

VkDescriptorSetLayout IVRMaterial::GetDescriptorSetLayout()
{
	return DescriptorSetLayout_;
}

void IVRMaterial::AssignDescriptorSet(VkDescriptorSet descriptor_set)
{
	DescriptorSet_ = descriptor_set;
}

VkDescriptorSet IVRMaterial::GetDescriptorSet()
{
	return DescriptorSet_;
}

void IVRMaterial::WriteToDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> descriptor_writes;

	//write the mvp matrix uniform buffer to the descriptor set
	VkDescriptorBufferInfo mvp_matrix_buffer_info{};
	mvp_matrix_buffer_info.buffer = MVPMatrixUB_->GetBuffer();
	mvp_matrix_buffer_info.offset = 0;
	mvp_matrix_buffer_info.range = MVPMatrixUB_->GetBufferSize();
	
	VkWriteDescriptorSet mvp_matrix_write{};
	mvp_matrix_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	mvp_matrix_write.dstSet = DescriptorSet_;
	mvp_matrix_write.dstBinding = 0;
	mvp_matrix_write.dstArrayElement = 0;
	mvp_matrix_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_write.descriptorCount = 1;
	mvp_matrix_write.pBufferInfo = &mvp_matrix_buffer_info;
	
	descriptor_writes.push_back(mvp_matrix_write);

	//write the texture samplers to the descriptor set

	for (int i = 0; i < Textures_.size(); i++)
	{
		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = Textures_[i]->GetTextureImageView();
		image_info.sampler = Textures_[i]->GetTextureSampler();

		VkWriteDescriptorSet texture_write{};
		texture_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		texture_write.dstSet = DescriptorSet_;
		texture_write.dstBinding = 1;
		texture_write.dstArrayElement = 0;
		texture_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texture_write.descriptorCount = 1;
		texture_write.pImageInfo = &image_info;

		descriptor_writes.push_back(texture_write);
	}
	
	vkUpdateDescriptorSets(DeviceManager_->GetLogicalDevice(), static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
}

void IVRMaterial::WriteMVPMatrixToDescriptorSet()
{
	VkDescriptorBufferInfo mvp_matrix_buffer_info{}; 
	mvp_matrix_buffer_info.buffer = MVPMatrixUB_->GetBuffer(); 
	mvp_matrix_buffer_info.offset = 0; 
	mvp_matrix_buffer_info.range = MVPMatrixUB_->GetBufferSize();

	VkWriteDescriptorSet mvp_matrix_write{}; 
	mvp_matrix_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
	mvp_matrix_write.dstSet = DescriptorSet_; 
	mvp_matrix_write.dstBinding = 0; 
	mvp_matrix_write.dstArrayElement = 0; 
	mvp_matrix_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
	mvp_matrix_write.descriptorCount = 1; 
	mvp_matrix_write.pBufferInfo = &mvp_matrix_buffer_info;

	vkUpdateDescriptorSets(DeviceManager_->GetLogicalDevice(), 1, &mvp_matrix_write, 0, nullptr);
}

void IVRMaterial::SetMVPMatrixUB(std::shared_ptr<IVRUBManager> mvp_matrix_ub)
{
	MVPMatrixUB_ = mvp_matrix_ub;
}

std::shared_ptr<IVRUBManager> IVRMaterial::GetMVPMatrixUB()
{
	return MVPMatrixUB_;
}

void IVRMaterial::SetPipeline(VkPipeline pipeline)
{
	Pipeline_ = pipeline;
}

VkPipeline IVRMaterial::GetPipeline()
{
	return Pipeline_;
}

void IVRMaterial::SetPipelineLayout(VkPipelineLayout pipeline_layout)
{
	PipelineLayout_ = pipeline_layout;
}

VkPipelineLayout IVRMaterial::GetPipelineLayout()
{
	return PipelineLayout_;
}

std::string IVRMaterial::GetVertexShaderPath()
{
	return VertexShaderPath_;
}

std::string IVRMaterial::GetFragmentShaderPath()
{
	return FragmentShaderPath_;
}