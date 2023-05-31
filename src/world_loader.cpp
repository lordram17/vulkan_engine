#include "world_loader.h"
#include "ivr_path.h"

#include <fstream>
#include <string>


IVRWorldLoader::IVRWorldLoader(std::shared_ptr<IVRDeviceManager> device_manager, std::shared_ptr<IVRLightManager> light_manager, 
								std::shared_ptr<IVRCamera> camera, uint32_t swapchain_image_count) :
	DeviceManager_(device_manager), LightManager_(light_manager), SwapchainImageCount_(swapchain_image_count), Camera_(camera)
{
}

IVRWorldLoader::~IVRWorldLoader()
{
}

std::vector<std::shared_ptr<IVRBaseMaterial>> IVRWorldLoader::LoadBaseMaterialsFromJson()
{
	std::string base_materials_path = IVRPath::GetCrossPlatformPath({ "scene", "base_materials.json" });
	std::ifstream base_materials_file = std::ifstream(base_materials_path);
	nlohmann::json base_materials_json_data = nlohmann::json::parse(base_materials_file);

	std::vector<std::shared_ptr<IVRBaseMaterial>> base_materials;

	for (uint32_t i = 0; i < base_materials_json_data.size(); i++)
	{
		nlohmann::json base_material = base_materials_json_data[i];
		
		std::string name = base_material["name"];
		std::string vertex_shader_path = base_material["vertex_shader"];
		std::string fragment_shader_path = base_material["fragment_shader"];
		uint32_t texture_count = base_material["texture_count"];
		std::string default_texture = base_material["default_texture"];
		bool is_cubemap = name == "cubemap";

		std::shared_ptr<IVRBaseMaterial> material = std::make_shared<IVRBaseMaterial>(name, vertex_shader_path, fragment_shader_path, default_texture, 
																						LightManager_->GetLightCount(), texture_count, SwapchainImageCount_, is_cubemap);
		base_materials.push_back(material);
		NameBaseMaterialMap_[name] = material;
	}

	return base_materials;

}

std::vector<std::shared_ptr<IVRRenderObject>>  IVRWorldLoader::LoadRenderObjectsFromJson()
{
	std::vector<std::shared_ptr<IVRRenderObject>> render_objects;

	std::string objects_path = IVRPath::GetCrossPlatformPath({"scene", "objects.json"});

	std::ifstream object_file(objects_path);
	nlohmann::json objects_json_data = nlohmann::json::parse(object_file);

	for (uint32_t i = 0; i < objects_json_data.size(); i++)
	{
		nlohmann::json object = objects_json_data[i];

		std::shared_ptr<IVRModel> model;
		std::shared_ptr<IVRMaterialInstance> material;
		std::shared_ptr<IVRRenderObject> render_object;

		if (object["type"] == "3d_model" || object["type"] == "skybox")
		{
			std::string name = object["name"];
			std::string model_path = object["model_path"];
			model = std::make_shared<IVRModel>(DeviceManager_, name, model_path);

			model->SetPosition(glm::vec3(object["transform"]["position"][0], object["transform"]["position"][1], object["transform"]["position"][2]));
			model->SetRotation(glm::vec3(object["transform"]["rotation"][0], object["transform"]["rotation"][1], object["transform"]["rotation"][2]));
			model->SetScale(glm::vec3(object["transform"]["scale"][0], object["transform"]["scale"][1], object["transform"]["scale"][2]));
			
			std::string material_name = object["material"];

			nlohmann::json material_properties = object["material_properties"];
			//textures
			std::vector<std::string> texture_names = material_properties["textures"];
			if (texture_names.size() == 0)
			{
				texture_names.push_back(NameBaseMaterialMap_[material_name]->GetDefaultTexture());
			}
			//material properties
			MaterialPropertiesUBObj material_properties_ubobj;

			if (material_name == "cubemap")
			{
				material_properties_ubobj.IsCubemap = true;
			}
			else if (material_name == "blinn-phong")
			{
				material_properties_ubobj.SpecularColor = glm::vec3(material_properties["specular_color"][0], 
																	material_properties["specular_color"][1], material_properties["specular_color"][2]);
				material_properties_ubobj.DiffuseColor = glm::vec3(material_properties["diffuse_color"][0], 
																	material_properties["diffuse_color"][1], material_properties["diffuse_color"][2]);
				material_properties_ubobj.SpecularPower = material_properties["specular_power"];
			}

			material = std::make_shared<IVRMaterialInstance>(DeviceManager_, NameBaseMaterialMap_[material_name], texture_names, material_properties_ubobj, SwapchainImageCount_,
														LightManager_->GetAllLightUBs());
			

			if (model != nullptr && material != nullptr) {
				render_object = std::make_shared<IVRRenderObject>(model, material, Camera_, SwapchainImageCount_);
				render_objects.push_back(render_object);
			}
			else {
				if (model == nullptr)
				{
					IVR_LOG_ERROR("Could not find model at path : " + model_path);
				}

				if (material == nullptr)
				{
					IVR_LOG_ERROR("Could not find the material " + material_name + "for model at path : " + model_path);
				}
			}
		}
	}

	return render_objects;
}

std::vector<IVRLight>&& IVRWorldLoader::LoadLightsFromJson()
{
	std::string lights_path = IVRPath::GetCrossPlatformPath({ "scene", "lights.json" });

	std::ifstream lights_file(lights_path);
	nlohmann::json lights_json_data = nlohmann::json::parse(lights_file);

	for (uint32_t i = 0; i < lights_json_data.size(); i++)
	{
		nlohmann::json light_data = lights_json_data[i];

		IVRLight light = {};

		light.Direction = glm::vec3(light_data["direction"][0], light_data["direction"][1], light_data["direction"][2]);
		light.Position = glm::vec3(light_data["position"][0], light_data["position"][1], light_data["position"][2]);

		light.AmbientColor = glm::vec3(light_data["ambient_color"][0], light_data["ambient_color"][1], light_data["ambient_color"][2]);
		light.DiffuseColor = glm::vec3(light_data["diffuse_color"][0], light_data["diffuse_color"][1], light_data["diffuse_color"][2]);
		light.SpecularColor = glm::vec3(light_data["specular_color"][0], light_data["specular_color"][1], light_data["specular_color"][2]);

		Lights_.push_back(light);
	}

	return std::move(Lights_);
}


