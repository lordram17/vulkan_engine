#include "material_instance.h"

IVRMaterialInstance::IVRMaterialInstance(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRBaseMaterial> base_material,
	std::vector<std::string> texture_names, MaterialPropertiesUBObj properties, uint32_t swapchain_image_count,
	std::vector<std::vector<std::shared_ptr<IVRUBManager>>>& light_ubos) :
	DeviceManager_(device_manager), BaseMaterial_(base_material),
	MaterialProperties_(properties), SwapchainImageCount_(swapchain_image_count), LightUBs_(light_ubos)
{
	for (std::string texture_name : texture_names)
	{
		std::shared_ptr<IVRTexture> texture_object;

		if (MaterialProperties_.IsCubemap)
		{
			if (TextureNames_.size() > 1) {
				IVR_LOG_ERROR("Loading more than one cubemap is not supported yet");
			}

			IVR_LOG_INFO("Loading cubemap texture: " + texture_name);

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
}

void IVRMaterialInstance::AssignDepthTextures(std::vector<std::shared_ptr<IVRTextureDepth>> depth_textures)
{
	DepthTextures_ = depth_textures;
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
	
	std::vector<VkDescriptorBufferInfo> light_buffer_infos;
	//wrie the light uniform buffer to the descriptor set
	for (uint32_t i = 0; i < LightCount_; i++) {
		VkDescriptorBufferInfo light_buffer_info{};
		light_buffer_info.buffer = LightUBs_[swapchain_image_index][i]->GetBuffer();
		light_buffer_info.offset = 0;
		light_buffer_info.range = LightUBs_[swapchain_image_index][i]->GetBufferSize();
		light_buffer_infos.push_back(light_buffer_info);
		
		VkWriteDescriptorSet light_write{};
		light_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		light_write.dstSet = DescriptorSets_[swapchain_image_index];
		light_write.dstBinding = 1 + i;
		light_write.dstArrayElement = 0;
		light_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		light_write.descriptorCount = 1;
		light_write.pBufferInfo = &light_buffer_infos[i];
		
		descriptor_writes.push_back(light_write);
	}

	std::vector<VkDescriptorBufferInfo> light_mvp_buffer_infos;
	//write the light mvp matrix uniform buffer to the descriptor set
	for (uint32_t i = 0; i < LightCount_; i++) {
		
		VkDescriptorBufferInfo light_mvp_buffer_info{};
		light_mvp_buffer_info.buffer = LightMVPUBManagers_[swapchain_image_index]->GetBuffer();
		light_mvp_buffer_info.offset = 0;
		light_mvp_buffer_info.range = LightMVPUBManagers_[swapchain_image_index]->GetBufferSize();
		light_mvp_buffer_infos.push_back(light_mvp_buffer_info);

		VkWriteDescriptorSet light_mvp_write{};
		light_mvp_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		light_mvp_write.dstSet = DescriptorSets_[swapchain_image_index];
		light_mvp_write.dstBinding = 1 + LightCount_ + i;
		light_mvp_write.dstArrayElement = 0;
		light_mvp_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		light_mvp_write.descriptorCount = 1;
		light_mvp_write.pBufferInfo = &light_mvp_buffer_infos[i];

		descriptor_writes.push_back(light_mvp_write);
	}


	std::vector<VkDescriptorImageInfo> depth_texture_image_infos;
	for (uint32_t i = 0; i < LightCount_; i++)
	{
		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		image_info.imageView = DepthTextures_[swapchain_image_index]->GetTextureImageView();
		image_info.sampler = DepthTextures_[swapchain_image_index]->GetTextureSampler();
		depth_texture_image_infos.push_back(image_info);

		VkWriteDescriptorSet depth_texture_write{};
		depth_texture_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		depth_texture_write.dstSet = DescriptorSets_[swapchain_image_index];
		depth_texture_write.dstBinding = 2 * LightCount_ + 1 + i;
		depth_texture_write.dstArrayElement = 0;
		depth_texture_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		depth_texture_write.descriptorCount = 1;
		depth_texture_write.pImageInfo = &depth_texture_image_infos[i];

		descriptor_writes.push_back(depth_texture_write);
	}

	//write the material properties uniform buffer to the descriptor set
	VkDescriptorBufferInfo material_properties_buffer_info{};
	material_properties_buffer_info.buffer = MaterialPropertiesUBs_[swapchain_image_index]->GetBuffer();
	material_properties_buffer_info.offset = 0;
	material_properties_buffer_info.range = MaterialPropertiesUBs_[swapchain_image_index]->GetBufferSize();

	VkWriteDescriptorSet material_properties_write{};
	material_properties_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	material_properties_write.dstSet = DescriptorSets_[swapchain_image_index];
	material_properties_write.dstBinding = 3 * LightCount_ + 1;
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
		texture_write.dstBinding = 3 * LightCount_ + 2 + i;
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

void IVRMaterialInstance::AssignLightMVPUniformBuffers(std::shared_ptr<IVRUBManager> light_mvp_ubos)
{
	LightMVPUBManagers_.push_back(light_mvp_ubos);
}

std::shared_ptr<IVRBaseMaterial> IVRMaterialInstance::GetBaseMaterial()
{
	return BaseMaterial_;
}
