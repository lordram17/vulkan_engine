#include "material_instance.h"

IVRMaterialInstance::IVRMaterialInstance(std::shared_ptr<IVRDeviceManager> device_manager, 
	std::string vertex_shader_name, std::string fragment_shader_name, 
	std::vector<std::string> texture_names, MaterialPropertiesUBObj properties, uint32_t swapchain_image_count,
	std::vector<std::vector<std::shared_ptr<IVRUBManager>>>& light_ubos) :
	DeviceManager_(device_manager), 
	MaterialProperties_(properties), SwapchainImageCount_(swapchain_image_count), LightUBs_(light_ubos)
{
	VertexShaderPath_ = IVRPath::GetCrossPlatformPath({"shaders", vertex_shader_name });
	FragmentShaderPath_ = IVRPath::GetCrossPlatformPath({ "shaders", fragment_shader_name });

	for (std::string texture_name : texture_names)
	{
		std::shared_ptr<IVRTexture> texture_object;

		if (MaterialProperties_.IsCubemap)
		{
			if (TextureNames_.size() > 1) {
				IVR_LOG_ERROR("Loading more than one cubemap is not supported yet");
			}

			IVR_LOG_INFO("Loading cubemap: " + texture_name);

			std::string texture_path = IVRPath::GetCrossPlatformPath({ "texture_files/cubemaps", texture_name });
			texture_object = std::make_shared<IVRTextureCube>(DeviceManager_, texture_path);
			Textures_.push_back(texture_object);
		}
		else {

			IVR_LOG_INFO("Loading 2D texture: " + texture_name);

			std::string texture_path = IVRPath::GetCrossPlatformPath({ "texture_files", texture_name });
			texture_object = std::make_shared<IVRTexture2D>(DeviceManager_, texture_path);
		}

		Textures_.push_back(texture_object);
	}

	LightCount_ = LightUBs_[0].size();
	InitMVPMatrixUBs();
	InitMaterialPropertiesUBs();
	CreateDescriptorSetLayoutInfo();
}

void IVRMaterialInstance::CreateDescriptorSetLayoutInfo()
{
	//binding layout :
	//0: MVP matrix
	//1-5: Light 1-5
	//6 : Material properties
	// 7+ : textures

	DescriptorSetInfo_ = {};
	
	uint32_t binding_count = 0;

	//assign the mvp matrix uniform buffer always to the binding 0
	VkDescriptorSetLayoutBinding mvp_matrix_binding{};
	mvp_matrix_binding.binding = binding_count;
	mvp_matrix_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_binding.descriptorCount = 1;
	mvp_matrix_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvp_matrix_binding.pImmutableSamplers = nullptr; // Optional

	DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(mvp_matrix_binding);
	binding_count++;

	//adding 5 light bindings
	while (binding_count < LightCount_ + 1) {
		//Light at binding 1
		VkDescriptorSetLayoutBinding light_binding{};
		light_binding.binding = binding_count;
		light_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		light_binding.descriptorCount = 1;
		light_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		light_binding.pImmutableSamplers = nullptr; // Optional

		DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(light_binding);
		binding_count++;
	}
	
	//adding the material properties binding
	VkDescriptorSetLayoutBinding material_properties_binding{}; 
	material_properties_binding.binding = binding_count;
	material_properties_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
	material_properties_binding.descriptorCount = 1;
	material_properties_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	material_properties_binding.pImmutableSamplers = nullptr; // Optional

	DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(material_properties_binding);
	binding_count++;

	//assign the texture samplers to the next bindings
	for (std::shared_ptr<IVRTexture> texture_object : Textures_)
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

	int a = 1;
}

IVRDescriptorSetInfo IVRMaterialInstance::GetDescriptorSetInfo()
{
	return DescriptorSetInfo_;
}

std::vector<VkDescriptorPoolSize> IVRMaterialInstance::GetDescriptorPoolSize()
{
	std::vector<VkDescriptorPoolSize> descriptor_pool_size;
	
	//the mvp matrix size is always 1
	VkDescriptorPoolSize mvp_matrix_pool_size{};
	mvp_matrix_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_pool_size.descriptorCount = 1;
	
	VkDescriptorPoolSize light_pool_size{};
	light_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	light_pool_size.descriptorCount = 5;

	VkDescriptorPoolSize material_properties_pool_size{};
	material_properties_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	material_properties_pool_size.descriptorCount = 1;
	
	//the texture samplers may be more than one dependending on the number of textures
	VkDescriptorPoolSize texture_pool_size{};
	texture_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texture_pool_size.descriptorCount = Textures_.size();
	
	for(uint32_t i = 0; i < SwapchainImageCount_; i++){
		descriptor_pool_size.push_back(mvp_matrix_pool_size);
		descriptor_pool_size.push_back(texture_pool_size);
		descriptor_pool_size.push_back(material_properties_pool_size);
		descriptor_pool_size.push_back(light_pool_size);
	}

	return descriptor_pool_size;
}

void IVRMaterialInstance::AssignDescriptorSetLayout(VkDescriptorSetLayout layout)
{
	DescriptorSetLayout_ = layout;
}

VkDescriptorSetLayout IVRMaterialInstance::GetDescriptorSetLayout()
{
	return DescriptorSetLayout_;
}

void IVRMaterialInstance::AssignDescriptorSet(VkDescriptorSet descriptor_set)
{
	DescriptorSets_.push_back(descriptor_set);

	if (DescriptorSets_.size() > SwapchainImageCount_)
	{
		IVR_LOG_ERROR("The number of descriptor sets is greater than the number of swapchain images");
		throw std::runtime_error("The number of descriptor sets is greater than the number of swapchain images");
	}
}

VkDescriptorSet IVRMaterialInstance::GetDescriptorSet(uint32_t swapchain_image_index)
{
	return DescriptorSets_[swapchain_image_index];
}

void IVRMaterialInstance::WriteToDescriptorSet(uint32_t swapchain_image_index)
{
	std::vector<VkWriteDescriptorSet> descriptor_writes;

	//write the mvp matrix uniform buffer to the descriptor set
	VkDescriptorBufferInfo mvp_matrix_buffer_info{};
	mvp_matrix_buffer_info.buffer = MVPMatrixUBs_[swapchain_image_index]->GetBuffer();
	mvp_matrix_buffer_info.offset = 0;
	mvp_matrix_buffer_info.range = MVPMatrixUBs_[swapchain_image_index]->GetBufferSize();
	
	VkWriteDescriptorSet mvp_matrix_write{};
	mvp_matrix_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	mvp_matrix_write.dstSet = DescriptorSets_[swapchain_image_index];
	mvp_matrix_write.dstBinding = 0;
	mvp_matrix_write.dstArrayElement = 0;
	mvp_matrix_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_write.descriptorCount = 1;
	mvp_matrix_write.pBufferInfo = &mvp_matrix_buffer_info;
	
	descriptor_writes.push_back(mvp_matrix_write);

	//wrie the light uniform buffer to the descriptor set
	for (uint32_t i = 0; i < LightCount_; i++) {
		VkDescriptorBufferInfo light_buffer_info{};
		light_buffer_info.buffer = LightUBs_[swapchain_image_index][i]->GetBuffer();
		light_buffer_info.offset = 0;
		light_buffer_info.range = LightUBs_[swapchain_image_index][i]->GetBufferSize();
		
		VkWriteDescriptorSet light_write{};
		light_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		light_write.dstSet = DescriptorSets_[swapchain_image_index];
		light_write.dstBinding = i + 1;
		light_write.dstArrayElement = 0;
		light_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		light_write.descriptorCount = 1;
		light_write.pBufferInfo = &light_buffer_info;
		
		descriptor_writes.push_back(light_write);
	}

	//write the material properties uniform buffer to the descriptor set
	VkDescriptorBufferInfo material_properties_buffer_info{};
	material_properties_buffer_info.buffer = MaterialPropertiesUBs_[swapchain_image_index]->GetBuffer();
	material_properties_buffer_info.offset = 0;
	material_properties_buffer_info.range = MaterialPropertiesUBs_[swapchain_image_index]->GetBufferSize();

	VkWriteDescriptorSet material_properties_write{};
	material_properties_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	material_properties_write.dstSet = DescriptorSets_[swapchain_image_index];
	material_properties_write.dstBinding = LightCount_ + 1;
	material_properties_write.dstArrayElement = 0;
	material_properties_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	material_properties_write.descriptorCount = 1;
	material_properties_write.pBufferInfo = &material_properties_buffer_info;

	descriptor_writes.push_back(material_properties_write);

	//write the texture samplers to the descriptor set
	for (int i = 0; i < Textures_.size(); i++)
	{
		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = Textures_[i]->GetTextureImageView();
		image_info.sampler = Textures_[i]->GetTextureSampler();

		VkWriteDescriptorSet texture_write{};
		texture_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		texture_write.dstSet = DescriptorSets_[swapchain_image_index];
		texture_write.dstBinding = LightCount_ + 2 + i;
		texture_write.dstArrayElement = 0;
		texture_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texture_write.descriptorCount = 1;
		texture_write.pImageInfo = &image_info;

		descriptor_writes.push_back(texture_write);
	}
	
	vkUpdateDescriptorSets(DeviceManager_->GetLogicalDevice(), static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
}

void IVRMaterialInstance::WriteMVPMatrixToDescriptorSet(uint32_t swapchain_image_index)
{
	VkDescriptorBufferInfo mvp_matrix_buffer_info{}; 
	mvp_matrix_buffer_info.buffer = MVPMatrixUBs_[swapchain_image_index]->GetBuffer();
	mvp_matrix_buffer_info.offset = 0; 
	mvp_matrix_buffer_info.range = MVPMatrixUBs_[swapchain_image_index]->GetBufferSize();

	VkWriteDescriptorSet mvp_matrix_write{}; 
	mvp_matrix_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
	mvp_matrix_write.dstSet = DescriptorSets_[swapchain_image_index];
	mvp_matrix_write.dstBinding = 0; 
	mvp_matrix_write.dstArrayElement = 0; 
	mvp_matrix_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
	mvp_matrix_write.descriptorCount = 1; 
	mvp_matrix_write.pBufferInfo = &mvp_matrix_buffer_info;

	vkUpdateDescriptorSets(DeviceManager_->GetLogicalDevice(), 1, &mvp_matrix_write, 0, nullptr);
}

void IVRMaterialInstance::InitMVPMatrixUBs()
{
	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		std::shared_ptr<IVRUBManager> mvp_matrix_ub = std::make_shared<IVRUBManager>(DeviceManager_, sizeof(MVPUBObj));
		MVPMatrixUBs_.push_back(mvp_matrix_ub);
	}
}

std::shared_ptr<IVRUBManager> IVRMaterialInstance::GetMVPMatrixUB(uint32_t swapchain_image_index)
{
	return MVPMatrixUBs_[swapchain_image_index];
}

void IVRMaterialInstance::SetLightsUBs(std::vector<std::shared_ptr<IVRUBManager>> light_ubs)
{
	LightUBs_.push_back(light_ubs);
	LightCount_ = light_ubs.size();
}

void IVRMaterialInstance::InitMaterialPropertiesUBs()
{
	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		std::shared_ptr<IVRUBManager> material_properties_ub = std::make_shared<IVRUBManager>(DeviceManager_, sizeof(MaterialPropertiesUBObj));

		//copy the material properties to the buffer
		material_properties_ub->WriteToUniformBuffer(&MaterialProperties_, sizeof(MaterialPropertiesUBObj));

		MaterialPropertiesUBs_.push_back(material_properties_ub);
	}
}

void IVRMaterialInstance::SetPipeline(VkPipeline pipeline)
{
	Pipeline_ = pipeline;
}

VkPipeline IVRMaterialInstance::GetPipeline()
{
	return Pipeline_;
}

void IVRMaterialInstance::SetPipelineLayout(VkPipelineLayout pipeline_layout)
{
	PipelineLayout_ = pipeline_layout;
}

VkPipelineLayout IVRMaterialInstance::GetPipelineLayout()
{
	return PipelineLayout_;
}

std::string IVRMaterialInstance::GetVertexShaderPath()
{
	return VertexShaderPath_;
}

std::string IVRMaterialInstance::GetFragmentShaderPath()
{
	return FragmentShaderPath_;
}

void IVRMaterialInstance::UpdatePipelineConfigBasedOnMaterialProperties(IVRFixedFunctionPipelineConfig& ff_pipeline_config)
{
	if (MaterialProperties_.IsCubemap)
	{
		ff_pipeline_config.Rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	}
}
