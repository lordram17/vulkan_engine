#include "world.h"

IVRWorld::IVRWorld(std::shared_ptr<IVRDeviceManager> device_manager, uint32_t swapchain_image_count) :
	DeviceManager_(device_manager), SwapchainImageCount_(swapchain_image_count)
{
}

//runs in the beginning
void IVRWorld::Init()
{
	IVR_LOG_INFO("Initializing world...");
	
	DescriptorManager_ = std::make_shared<IVRDescriptorManager>(DeviceManager_);
	SMDescriptorManager_ = std::make_shared<IVRDescriptorManager>(DeviceManager_);
	LightManager_ = std::make_shared<IVRLightManager>(DeviceManager_, SwapchainImageCount_);

	SetupCamera();
	
	IVRWorldLoader world_loader(DeviceManager_, LightManager_, Camera_, SwapchainImageCount_);

	LightManager_->SetupLights(world_loader.LoadLightsFromJson());
	BaseMaterials_ = world_loader.LoadBaseMaterialsFromJson();
	RenderObjects_ = world_loader.LoadRenderObjectsFromJson();
	OrganizeRenderObjectsByBaseMaterial();
	
	CreateDescriptorSetLayoutsForBaseMaterials();
	CountPoolSizes();
	CreateDescriptorSets();

	//shadow mapper in the engine is instantiated post world init so the shader map materials can only be created after the world init
}

void IVRWorld::PostShadowMapperInit()
{
	AssignDescriptorSetLayoutToShadowMapMaterials();
	CreateShadowMapMaterialDescriptorSets();
	WriteShadowMapMaterialDescriptorSets();

	//sharing light mvp uniform buffer from shadow map materials to base materials
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		render_object->AssignLightMVPUBToMaterialInstance();
	}
	WriteDescriptorSets(); //the material descriptor sets (which also include the depth texture from the shadow map)
}

//runs every frame
void IVRWorld::Update(float dt, uint32_t swapchain_index)
{
	Camera_->MoveCamera(dt);
	LightManager_->TransformLightsByViewMatrix(Camera_->GetViewMatrix(), swapchain_index);
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		render_object->UpdateMVPMatrixUB(swapchain_index);
		render_object->UpdateLightMVPUB(swapchain_index, LightManager_->GetLight(0).GetLightView(), 
			LightManager_->GetLight(0).GetLightProjection(Camera_->FieldOfView, Camera_->AspectRatio, Camera_->NearPlane, Camera_->FarPlane));
	}
}


std::shared_ptr<IVRLightManager> IVRWorld::GetLightManager()
{
	return LightManager_;
}

std::vector<std::shared_ptr<IVRRenderObject>>& IVRWorld::GetRenderObjects()
{
	return RenderObjects_;
}

std::vector<std::shared_ptr<IVRBaseMaterial>>& IVRWorld::GetBaseMaterials()
{
	return BaseMaterials_;
}

void IVRWorld::OrganizeRenderObjectsByBaseMaterial()
{
	IVR_LOG_INFO("Organizing render objects by base material...");
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		std::shared_ptr<IVRBaseMaterial> base_material = render_object->GetMaterialInstance()->GetBaseMaterial();
		BaseMaterialRenderObjectMap_[base_material].push_back(render_object);
	}
}

std::unordered_map<std::shared_ptr<IVRBaseMaterial>, std::vector<std::shared_ptr<IVRRenderObject>>>& IVRWorld::GetBaseMaterialRenderObjectMap()
{
	return BaseMaterialRenderObjectMap_;
}

void IVRWorld::SetupCamera()
{
	Camera_ = std::make_shared<IVRCamera>();
}

void IVRWorld::SetCameraAspectRatio(float aspect_ratio)
{
	Camera_->AspectRatio = aspect_ratio;
}

void IVRWorld::CreateDescriptorSetLayoutsForBaseMaterials()
{
	IVR_LOG_INFO("Creating descriptor set layouts for render objects...");
	//take the IVRDescriptorSetInfo objects from the renderobject->material and create the descriptor set layouts
	//for each render object
		
	for (std::shared_ptr<IVRBaseMaterial>& base_material : BaseMaterials_)
	{
		IVRDescriptorSetInfo descriptor_set_info = base_material->GetDescriptorSetInfo();
		VkDescriptorSetLayout descriptor_set_layout = DescriptorManager_->CreateDescriptorSetLayout(descriptor_set_info);
		base_material->SetDescriptorSetLayout(descriptor_set_layout);
	}
}

std::vector<VkDescriptorPoolSize> IVRWorld::CountPoolSizes()
{
	IVR_LOG_INFO("Counting descriptor pool size for all descriptors...");
	std::vector<VkDescriptorPoolSize> pool_sizes;

	//for each render object
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		for (VkDescriptorPoolSize pool_size : render_object->GetMaterialInstance()->GetBaseMaterial()->GetDescriptorPoolSize())
		{
			pool_sizes.push_back(pool_size);
		}
	}

	//*IMP* Note about materials : looping over render objects to get materials is incorrect because the same material can be used by multiple render objects.
	
	return pool_sizes;
}

void IVRWorld::CreateDescriptorSets()
{
	IVR_LOG_INFO("Creating descriptor sets for all render objects...");
	//steps
	//1. create the descriptor pool
	//2. create the descriptor sets
	//3. assign the descriptor sets to the materials

	std::vector<VkDescriptorPoolSize> pool_sizes = CountPoolSizes();

	DescriptorManager_->CreateDescriptorPool(pool_sizes, RenderObjects_.size() * SwapchainImageCount_); 
	//*IMP* Note : this is incorrect because the same material can be used by multiple render objects. 
	//So the pool size should be calculated by looping over materials and not render objects
	
	//create descriptor set for each material
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		for (uint32_t i = 0; i < SwapchainImageCount_; i++) 
		{
			VkDescriptorSet descriptor_set = DescriptorManager_->CreateDescriptorSet(render_object->GetMaterialInstance()->GetBaseMaterial()->GetDescriptorSetLayout());
			render_object->GetMaterialInstance()->AssignDescriptorSet(descriptor_set);
		}
	}
}

void IVRWorld::WriteDescriptorSets()
{
	IVR_LOG_INFO("Writing descriptor sets for all render objects...");
	//for each render object
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		for (uint32_t i = 0; i < SwapchainImageCount_; i++)
		{
			render_object->GetMaterialInstance()->WriteToDescriptorSet(i);
		}
	}
}

void IVRWorld::InitShadowMapMaterials(IVRDescriptorSetInfo descriptor_set_info)
{
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		std::shared_ptr<IVRShadowmapMaterial> sm_mat = std::make_shared<IVRShadowmapMaterial>(DeviceManager_, descriptor_set_info, SwapchainImageCount_);
		render_object->AssignShadowmapMaterial(sm_mat);
	}
}

void IVRWorld::AssignDescriptorSetLayoutToShadowMapMaterials()
{
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		std::shared_ptr<IVRShadowmapMaterial> sm_mat = render_object->GetShadowmapMaterial();
		sm_mat->AssignDescriptorSetLayout(DescriptorManager_->CreateDescriptorSetLayout(sm_mat->GetDescriptorSetInfo()));
	}
}

std::vector<VkDescriptorPoolSize> IVRWorld::CountShadowMapMaterialPoolSize()
{
	std::vector<VkDescriptorPoolSize> pool_sizes;

	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		std::shared_ptr<IVRShadowmapMaterial> sm_mat = render_object->GetShadowmapMaterial();
		
		for (VkDescriptorPoolSize pool_size : sm_mat->GetDescriptorPoolSize())
		{
			pool_sizes.push_back(pool_size);
		}
	}
	return pool_sizes;
}

void IVRWorld::CreateShadowMapMaterialDescriptorSets()
{
	std::vector<VkDescriptorPoolSize> pool_sizes = CountShadowMapMaterialPoolSize();
	
	SMDescriptorManager_->CreateDescriptorPool(pool_sizes, RenderObjects_.size() * SwapchainImageCount_);
	
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		std::shared_ptr<IVRShadowmapMaterial> sm_mat = render_object->GetShadowmapMaterial();
		
		for (uint32_t i = 0; i < SwapchainImageCount_; i++)
		{
			VkDescriptorSet descriptor_set = SMDescriptorManager_->CreateDescriptorSet(sm_mat->GetDescriptorSetLayout());
			sm_mat->AssignDescriptorSet(descriptor_set);
		}
	}
}

void IVRWorld::WriteShadowMapMaterialDescriptorSets()
{
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		std::shared_ptr<IVRShadowmapMaterial> sm_mat = render_object->GetShadowmapMaterial();
		for (uint32_t i = 0; i < SwapchainImageCount_; i++)
		{
			sm_mat->WriteToDescriptorSet(i);
		}
	}
}

void IVRWorld::AssignShadowMapDepthTextures(std::vector<std::shared_ptr<IVRDepthImage>> depth_images)
{
	std::vector<std::shared_ptr<IVRTextureDepth>> depth_textures;

	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		depth_textures.push_back(std::make_shared<IVRTextureDepth>(DeviceManager_, depth_images[i]));
	}

	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		render_object->GetMaterialInstance()->AssignDepthTextures(depth_textures);
	}
}





