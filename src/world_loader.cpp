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

std::vector<std::shared_ptr<IVRRenderObject>>  IVRWorldLoader::LoadRenderObjectsFromJson()
{
	std::vector<std::shared_ptr<IVRRenderObject>> render_objects;

	std::string objects_path = IVRPath::GetCrossPlatformPath({"scene", "objects.json"});
	std::string materials_path = IVRPath::GetCrossPlatformPath({ "scene", "materials.json" });

	std::ifstream material_file(materials_path);
	nlohmann::json material_json_data = nlohmann::json::parse(material_file);

	std::ifstream object_file(objects_path);
	nlohmann::json objects_json_data = nlohmann::json::parse(object_file);

	for (uint32_t i = 0; i < objects_json_data.size(); i++)
	{
		nlohmann::json object = objects_json_data[i];

		std::shared_ptr<IVRModel> model;
		std::shared_ptr<IVRMaterial> material;
		std::shared_ptr<IVRRenderObject> render_object;

		if (object["type"] == "3d_model" || object["type"] == "skybox")
		{
			std::string name = object["name"];
			std::string model_path = object["model_path"];
			
			model = std::make_shared<IVRModel>(DeviceManager_, model_path);
			
			std::string material_name = object["material"];

			for (uint32_t j = 0; j < material_json_data.size(); j++)
			{
				nlohmann::json material_json = material_json_data[j];

				if (material_json["name"] == material_name)
				{
					//shader paths
					std::string vertex_shader_path = material_json["vertex_shader"];
					std::string fragment_shader_path = material_json["fragment_shader"];
					
					//textures
					std::vector<std::string> texture_names = material_json["textures"];

					//material properties
					MaterialPropertiesUBObj material_properties_ubobj;

					if (material_json["type"] == "cubemap")
					{
						material_properties_ubobj.IsCubemap = true;
					}
					else if (material_json["type"] == "blinn-phong")
					{
						material_properties_ubobj.SpecularColor = glm::vec3(material_json["specular_color"][0], material_json["specular_color"][1], material_json["specular_color"][2]);
						material_properties_ubobj.DiffuseColor = glm::vec3(material_json["diffuse_color"][0], material_json["diffuse_color"][1], material_json["diffuse_color"][2]);
						material_properties_ubobj.SpecularPower = material_json["specular_power"];
					}

					material = std::make_shared<IVRMaterial>(DeviceManager_, vertex_shader_path, fragment_shader_path, texture_names, material_properties_ubobj, SwapchainImageCount_,
																LightManager_->GetAllLightUBs());
				}
			}

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


