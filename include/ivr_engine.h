#include <vulkan/vulkan.h>
#include <memory>

#include "instance_setup.h"
#include "ivr_window.h"
#include "swapchain_manager.h"
#include "renderpass.h"
#include "depth_image.h"
#include "pipeline_creator.h"
#include "debug_logger_utils.h"
#include "world.h"
#include "pipeline_creator.h"
#include "framebuffer_manager.h"
#include "sync_objects_manager.h"
#include "command_buffer_manager.h"
#include "shadow_map.h"


class IVREngine {

private:
	std::shared_ptr<IVRInstanceManager> InstanceManager_;
	std::shared_ptr<IVRWindow> Window_;
	std::shared_ptr<IVRDeviceManager> DeviceManager_;
	std::shared_ptr<IVRSwapchainManager> SwapchainManager_;
	std::shared_ptr<IVRRenderpass> Renderpass_;
	std::shared_ptr<IVRDepthImage> DepthImage_;
	std::shared_ptr<IVRWorld> World_;
	std::shared_ptr<IVRPipelineCreator> PipelineCreator_;
	std::shared_ptr<IVRFramebufferManager> FramebufferManager_;
	std::shared_ptr<IVRSyncObjectsManager> SyncObjectsManager_;
	std::shared_ptr<IVRCBManager> CBManager_;
	std::shared_ptr<IVRShadowMap> ShadowMap_;

	uint32_t CurrentSwapchainImageIndex_;

public:
	IVREngine();
	~IVREngine() {};

	//initialize the various resources for the rendering engine
	void InitEngine();

	void CreateRenderpass();
	void CreatePipelines();
	void PostWorldInit();

	void DrawFrame();

	std::shared_ptr<IVRDeviceManager> GetDeviceManager() { return DeviceManager_; }
	std::shared_ptr<IVRWindow> GetWindow() { return Window_; }
	void SetWorld(std::shared_ptr<IVRWorld> world) { World_ = world; }

	uint32_t QueryForSwapchainIndex();
	std::shared_ptr<IVRSwapchainManager> GetSwapchainManager() { return SwapchainManager_; }
};