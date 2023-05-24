#include "world_loader.h"
#include "ivr_path.h"

#include <fstream>
#include <string>


void IVRWorldLoader::LoadFiles()
{
	std::string objects_path = IVRPath::GetCrossPlatformPath({"scene", "objects.json"});
	std::string materials_path = IVRPath::GetCrossPlatformPath({ "scene", "materials.json" });

	std::ifstream material_file(materials_path);
	nlohmann::json materials_json_data = nlohmann::json::parse(material_file);

	for (uint32_t i = 0; i < materials_json_data.size(); i++)
	{

	}

	std::ifstream object_file(objects_path);
	nlohmann::json objects_json_data = nlohmann::json::parse(object_file);

	for (uint32_t i = 0; i < objects_json_data.size(); i++)
	{


	}
	
}
