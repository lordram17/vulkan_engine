#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <memory>
#include "device_setup.h"


struct IVRRenderpassConfig {
	std::vector<VkAttachmentDescription> ColorAttachments;

	bool isUseDepthAttachment;
	VkAttachmentDescription DepthAttachment;
};

class IVRRenderpass {

private:
	IVRRenderpassConfig RenderpassConfig_;
	VkRenderPass Renderpass_;
	VkSubpassDescription SubpassDescription_;
	VkSubpassDependency SubpassDependency_;
	std::shared_ptr<IVRDeviceManager> DeviceManager_;

public:
	IVRRenderpass(IVRRenderpassConfig renderpass_config, std::shared_ptr<IVRDeviceManager> device_manager);
	~IVRRenderpass();

	void CreateRenderpass();

	void BeginRenderPass(VkCommandBuffer command_buffer, VkFramebuffer framebuffer, VkExtent2D extent);
	void EndRenderPass(VkCommandBuffer command_buffer);

	VkRenderPass GetRenderpass();

};

