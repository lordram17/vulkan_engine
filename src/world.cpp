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
	SetupCamera();
	LoadRenderObjectsVector();
	CreateDescriptorSetLayoutsForRenderObjects();
	CountPoolSizes();
	CreateDescriptorSets();
	WriteDescriptorSets();
}

//runs every frame
void IVRWorld::Update(float dt, uint32_t swapchain_index)
{
	Camera_->MoveCamera(dt);

	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		render_object->UpdateMVPMatrixUB(swapchain_index);
	}
}


std::vector<std::shared_ptr<IVRRenderObject>> IVRWorld::GetRenderObjects()
{
	return RenderObjects_;
}


void IVRWorld::LoadRenderObjectsVector()
{
	IVR_LOG_INFO("Creating render objects...");

	IVR_LOG_INFO("Creating mvp matric uniform buffers for all render objects...");
	std::vector<std::shared_ptr<IVRUBManager>> mvp_matrix_ubs;
	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		mvp_matrix_ubs.push_back(std::make_shared<IVRUBManager>(DeviceManager_, sizeof(MVPUniformBufferObject)));
	}

	IVR_LOG_INFO("	- Creating viking room...");
	//logic for creating render objects and adding them to the vector
	std::shared_ptr<IVRModel> model = std::make_shared<IVRModel>(DeviceManager_, "viking_room/viking_room.obj");
	model->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	std::vector<std::string> textures_names = { "viking_room.png" };
	MaterialProperties material_properties;
	std::shared_ptr<IVRMaterial> material = std::make_shared<IVRMaterial>(DeviceManager_, "simple_texture_mapped.vert.spv", "simple_texture_mapped.frag.spv", textures_names, material_properties, SwapchainImageCount_);
	std::shared_ptr<IVRRenderObject> render_object = std::make_shared<IVRRenderObject>(model, material, Camera_, SwapchainImageCount_);
	RenderObjects_.push_back(render_object);

	IVR_LOG_INFO("	- Creating cubemap...");
	model = std::make_shared<IVRModel>(DeviceManager_, "simple_cube.obj");
	model->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	textures_names = { "koln_wallraffplatz"}; //for cubemap, only folder name is needed
	material_properties.IsCubemap = true;
	material = std::make_shared<IVRMaterial>(DeviceManager_, "cubemap.vert.spv", "cubemap.frag.spv", textures_names, material_properties, SwapchainImageCount_);
	render_object = std::make_shared<IVRRenderObject>(model, material, Camera_, SwapchainImageCount_);
	RenderObjects_.push_back(render_object);

	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		for (std::shared_ptr<IVRRenderObject> ro : RenderObjects_)
		{
			ro->GetMaterial()->SetMVPMatrixUB(mvp_matrix_ubs[i]);
			ro->UpdateMVPMatrixUB(i);
		}
	}
}

void IVRWorld::SetupCamera()
{
	Camera_ = std::make_shared<IVRCamera>();
	Camera_->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
}

void IVRWorld::CreateDescriptorSetLayoutsForRenderObjects()
{
	IVR_LOG_INFO("Creating descriptor set layouts for render objects...");
	//take the IVRDescriptorSetInfo objects from the renderobject->material and create the descriptor set layouts

	//for each render object
		
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		IVRDescriptorSetInfo descriptor_set_info = render_object->GetMaterial()->GetDescriptorSetInfo();
		VkDescriptorSetLayout descriptor_set_layout = DescriptorManager_->CreateDescriptorSetLayout(descriptor_set_info);
		render_object->GetMaterial()->AssignDescriptorSetLayout(descriptor_set_layout);
	}

	//*IMP* Note about materials : looping over render objects to get materials is incorrect because the same material can be used by multiple render objects.
}

std::vector<VkDescriptorPoolSize> IVRWorld::CountPoolSizes()
{
	IVR_LOG_INFO("Counting descriptor pool size for all descriptors...");
	std::vector<VkDescriptorPoolSize> pool_sizes;

	//for each render object
	for (std::shared_ptr<IVRRenderObject> render_object : RenderObjects_)
	{
		for (VkDescriptorPoolSize pool_size : render_object->GetMaterial()->GetDescriptorPoolSize())
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
			VkDescriptorSet descriptor_set = DescriptorManager_->CreateDescriptorSet(render_object->GetMaterial()->GetDescriptorSetLayout());
			render_object->GetMaterial()->AssignDescriptorSet(descriptor_set);
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
			render_object->GetMaterial()->WriteToDescriptorSet(i);
		}
	}
}





