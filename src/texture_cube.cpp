#include "texture_cube.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "debug_logger_utils.h"

IVRTextureCube::IVRTextureCube(std::shared_ptr<IVRDeviceManager> device_manager, std::string cubemap_folder_path) :
	IVRTexture(device_manager)
{
	//the order of these paths is important
	CubemapPaths_.push_back(cubemap_folder_path + "/px.png");
	CubemapPaths_.push_back(cubemap_folder_path + "/nx.png");
	CubemapPaths_.push_back(cubemap_folder_path + "/py.png");
	CubemapPaths_.push_back(cubemap_folder_path + "/ny.png");
	CubemapPaths_.push_back(cubemap_folder_path + "/pz.png");
	CubemapPaths_.push_back(cubemap_folder_path + "/nz.png");

	ImageViewType_ = VK_IMAGE_VIEW_TYPE_CUBE;
	SamplerAddressMode_ = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	TextureFormat_ = VK_FORMAT_R8G8B8A8_SRGB;
	LayerCount_ = 6;

	InitTexture();
}

IVRTextureCube::~IVRTextureCube()
{
}

void IVRTextureCube::CreateTextureImage()
{
	int tex_width, tex_height, tex_channels;
	std::vector<stbi_uc*> pixel_ptrs;

	for (std::string path : CubemapPaths_)
	{
		stbi_uc* pixels = stbi_load(path.c_str(), &tex_width,
						&tex_height, &tex_channels, STBI_rgb_alpha);

		if (pixels == nullptr)
		{
			IVR_LOG_ERROR("Failed to load cubemap image at path " + path);
			throw std::runtime_error("Failed to load texture image!");
		}
		else {
			IVR_LOG_INFO("Loaded cubemap image at path " + path);
		}

		pixel_ptrs.push_back(pixels);
	}

	VkDeviceSize single_texture_size = tex_width * tex_height * tex_channels;
	VkDeviceSize image_size = single_texture_size * LayerCount_; //3 is the number of channels (we are forcing the image to only have rgb)
	

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	IVRBufferUtilities::Spawn(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(), image_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		staging_buffer, staging_buffer_memory);

	void* data;
	vkMapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, 0, image_size, 0, &data);

	for (uint64_t i = 0; i < pixel_ptrs.size(); i++)
	{
		//stbi_uc* is a pointer to an array of unsigned chars
		//incrementing a void pointer does not work because the compiler does not know how many bytes to increment by
		//so we cast the void pointer to a stbi_uc* pointer and then increment it
		uint64_t mem_address = reinterpret_cast<uint64_t>(data) + single_texture_size * i;
		
		memcpy(reinterpret_cast<void*>(mem_address), pixel_ptrs[i], static_cast<size_t>(single_texture_size));
		//free the memory allocated by stbi_load after copying
		stbi_image_free(pixel_ptrs[i]);
	}

	vkUnmapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory);

	IVRImageUtils::CreateImageAndBindMemory(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
		tex_width, tex_height, TextureFormat_, LayerCount_, //6 layers
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, //cube compatible flag is required for cube maps
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, TextureImage_, TextureImageMemory_);

	IVRImageUtils::TransitionImageLayout(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
		TextureImage_, TextureFormat_, LayerCount_,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
	IVRImageUtils::CopyBufferToImage(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(), 6,
		staging_buffer, TextureImage_, tex_width, tex_height);

	IVRImageUtils::TransitionImageLayout(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
		TextureImage_, TextureFormat_, LayerCount_,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), staging_buffer, nullptr);
	vkFreeMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, nullptr);
}







