#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "descriptors.h"
#include "device_setup.h"
#include "renderobject.h"
#include "camera.h"
#include "debug_logger_utils.h"


//there should be only one world. Render objects can be grouped together into a scene (for now doing it directly)
class IVRWorld {

private:
	std::shared_ptr <IVRDescriptorManager> DescriptorManager_; //this class creates it own descriptor manager
	std::shared_ptr<IVRDeviceManager> DeviceManager_;

	std::vector<std::shared_ptr<IVRRenderObject>> RenderObjects_;
	std::shared_ptr<IVRCamera> Camera_;

public:

	IVRWorld(std::shared_ptr<IVRDeviceManager> device_manager);

	void SetupCamera();

	void LoadRenderObjectsVector();
	void CreateDescriptorSetLayoutsForRenderObjects();
	std::vector<VkDescriptorPoolSize> CountPoolSizes();
	void CreateDescriptorSets();
	void WriteDescriptorSets();

	void Init();
	void Update(float dt);

	std::vector<std::shared_ptr<IVRRenderObject>> GetRenderObjects();

};