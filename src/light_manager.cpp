#include "light_manager.h"

IVRLightManager::IVRLightManager(std::shared_ptr<IVRDeviceManager> device_manager, uint32_t swapchain_image_count) :
	DeviceManager_(device_manager), SwapchainImageCount_(swapchain_image_count)
{
}

void IVRLightManager::SetupLights(std::vector<IVRLight>&& lights)
{
	Lights_ = lights;

	for (uint32_t i = 0; i < SwapchainImageCount_; i++)
	{
		std::vector<std::shared_ptr<IVRUBManager>> light_ub_managers;
		std::vector<LightUBObj> light_ubos;
		for (uint32_t j = 0; j < Lights_.size(); j++)
		{
			LightUBObj light_ubo(Lights_[j]);
			light_ubos.push_back(light_ubo);
			
			std::shared_ptr<IVRUBManager> light_ub_manager = std::make_shared<IVRUBManager>(DeviceManager_, (VkDeviceSize)sizeof(LightUBObj));
			light_ub_manager->WriteToUniformBuffer(&light_ubo, (VkDeviceSize)sizeof(LightUBObj));
			light_ub_managers.push_back(light_ub_manager);
		}

		LightUBOs_.push_back(light_ubos);
		LightUBManagers_.push_back(light_ub_managers);
	}
}

void IVRLightManager::TransformLightsByViewMatrix(glm::mat4 view, uint32_t swapchain_index)
{
	std::vector<std::shared_ptr<IVRUBManager>>& light_ub_managers = LightUBManagers_[swapchain_index];
	std::vector<LightUBObj>& light_ubos = LightUBOs_[swapchain_index];

	for (uint32_t i = 0; i < Lights_.size(); i++)
	{
		IVRLight& light = Lights_[i];
		LightUBObj& light_ubo = light_ubos[i];
		
		switch (light.Type)
		{
		case IVRLightType::Directional:
			light_ubo.Direction = glm::vec3(view * glm::vec4(light.Direction, 0.0f));
			break;

		case IVRLightType::Point:
			light_ubo.Position = glm::vec3(view * glm::vec4(light.Position, 1.0f));
			break;
		case IVRLightType::Spot:
			light_ubo.Position = glm::vec3(view * glm::vec4(light.Position, 1.0f));
			light_ubo.Direction = glm::vec3(view * glm::vec4(light.Direction, 0.0f));
			break;
		default:
			IVR_LOG_ERROR("Light type not supported");
			throw std::runtime_error("Light type not supported");
			break;
		}

		light_ub_managers[i]->WriteToUniformBuffer(&light_ubo, (VkDeviceSize)sizeof(LightUBObj));
	}
}

uint32_t IVRLightManager::GetLightCount()
{
	return Lights_.size();
}

void IVRLightManager::UpdateLightByIndex(uint32_t light_index)
{
}

IVRLight& IVRLightManager::GetLightByIndex(uint32_t light_index)
{
	return Lights_[light_index];
}

std::shared_ptr<IVRUBManager> IVRLightManager::GetLightUBManagerByIndex(uint32_t light_index, uint32_t swapchain_index)
{
	return LightUBManagers_[swapchain_index][light_index];
}

std::vector<std::vector<std::shared_ptr<IVRUBManager>>>& IVRLightManager::GetAllLightUBs()
{
	return LightUBManagers_;
}


