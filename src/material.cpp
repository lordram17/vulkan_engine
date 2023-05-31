#include "material.h"
#include "ivr_path.h"

IVRBaseMaterial::IVRBaseMaterial(std::string name, std::string vertex_shader_path, std::string fragment_shader_path, std::string default_texture,
								uint32_t light_count, uint32_t texture_count, uint32_t swapchain_image_count, bool is_cubemap) :
	Name_(name), DefaultTexture_(default_texture),
	LightCount_(light_count), TextureCount_(texture_count), SwapchainImageCount_(swapchain_image_count), IsCubemap(is_cubemap)
{
	VertexShaderPath_ = IVRPath::GetCrossPlatformPath({"shaders", vertex_shader_path});
	FragmentShaderPath_ = IVRPath::GetCrossPlatformPath({"shaders", fragment_shader_path});

	CreateDescriptorSetLayoutInfo();
}

void IVRBaseMaterial::CreateDescriptorSetLayoutInfo()
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

	//adding light bindings
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

	while (binding_count < 2 * LightCount_ + 1)
	{
		VkDescriptorSetLayoutBinding light_mvp_binding{};
		light_mvp_binding.binding = binding_count;
		light_mvp_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		light_mvp_binding.descriptorCount = 1;
		light_mvp_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		light_mvp_binding.pImmutableSamplers = nullptr; // Optional

		DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(light_mvp_binding);
		binding_count++;
	}

	//adding depth textures
	while (binding_count < 3 * LightCount_ + 1 )
	{
		VkDescriptorSetLayoutBinding depth_texture_binding{};
		depth_texture_binding.binding = binding_count;
		depth_texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		depth_texture_binding.descriptorCount = 1;
		depth_texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		depth_texture_binding.pImmutableSamplers = nullptr; // Optional
		DescriptorSetInfo_.DescriptorSetLayoutBindings.push_back(depth_texture_binding);
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
	for (uint32_t i = 0; i < TextureCount_; i++)
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

IVRDescriptorSetInfo IVRBaseMaterial::GetDescriptorSetInfo()
{
	return DescriptorSetInfo_;
}

void IVRBaseMaterial::SetDescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout)
{
	DescriptorSetLayout_ = descriptor_set_layout;
}

VkDescriptorSetLayout IVRBaseMaterial::GetDescriptorSetLayout()
{
	return DescriptorSetLayout_;
}

std::vector<VkDescriptorPoolSize> IVRBaseMaterial::GetDescriptorPoolSize()
{
	std::vector<VkDescriptorPoolSize> descriptor_pool_size;

	//the mvp matrix size is always 1
	VkDescriptorPoolSize mvp_matrix_pool_size{};
	mvp_matrix_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvp_matrix_pool_size.descriptorCount = 1;

	VkDescriptorPoolSize light_pool_size{};
	light_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	light_pool_size.descriptorCount = LightCount_;

	VkDescriptorPoolSize light_mvp_pool_size{};
	light_mvp_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	light_mvp_pool_size.descriptorCount = LightCount_;

	VkDescriptorPoolSize depth_texture_pool_size{};
	depth_texture_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depth_texture_pool_size.descriptorCount = LightCount_;

	VkDescriptorPoolSize material_properties_pool_size{};
	material_properties_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	material_properties_pool_size.descriptorCount = 1;

	//the texture samplers may be more than one dependending on the number of textures
	VkDescriptorPoolSize texture_pool_size{};
	texture_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texture_pool_size.descriptorCount = TextureCount_;

	for (uint32_t i = 0; i < SwapchainImageCount_; i++) {
		descriptor_pool_size.push_back(mvp_matrix_pool_size);
		descriptor_pool_size.push_back(texture_pool_size);
		descriptor_pool_size.push_back(material_properties_pool_size);
		descriptor_pool_size.push_back(light_pool_size);
		descriptor_pool_size.push_back(light_mvp_pool_size);
		descriptor_pool_size.push_back(depth_texture_pool_size);
	}

	return descriptor_pool_size;
}

void IVRBaseMaterial::SetPipeline(VkPipeline pipeline)
{
	Pipeline_ = pipeline;
}

VkPipeline IVRBaseMaterial::GetPipeline()
{
	return Pipeline_;
}

void  IVRBaseMaterial::SetPipelineLayout(VkPipelineLayout pipeline_layout)
{
	PipelineLayout_ = pipeline_layout;
}

VkPipelineLayout  IVRBaseMaterial::GetPipelineLayout()
{
	return PipelineLayout_;
}

void IVRBaseMaterial::UpdatePipelineConfigBasedOnMaterialProperties(IVRFixedFunctionPipelineConfig& ff_pipeline_config)
{
	if (IsCubemap)
	{
		ff_pipeline_config.Rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	}
}

std::string  IVRBaseMaterial::GetVertexShaderPath()
{
	return VertexShaderPath_;
}

std::string  IVRBaseMaterial::GetFragmentShaderPath()
{
	return FragmentShaderPath_;
}

std::string IVRBaseMaterial::GetDefaultTexture()
{
	return DefaultTexture_;
}
