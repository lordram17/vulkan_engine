#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>

#include "singlecommand_utils.h"
#include "buffer_utils.h"

class IVRImageUtils {

public:

	static VkImageView CreateImageView(VkDevice logical_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo view_create_info = {};
		view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_create_info.image = image;
		view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // 2D texture
		view_create_info.format = format;
		view_create_info.subresourceRange.aspectMask = aspectFlags; // what kind of data will be stored in the image
		view_create_info.subresourceRange.baseMipLevel = 0; // start at base level
		view_create_info.subresourceRange.levelCount = 1; // only one level
		view_create_info.subresourceRange.baseArrayLayer = 0; // start at base layer
		view_create_info.subresourceRange.layerCount = 1; // only one layer

		//can add swizzle here to swizzle the image components

		VkImageView imageView;
		if (vkCreateImageView(logical_device, &view_create_info, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	static void CopyBufferToImage(VkDevice logical_device, uint32_t queue_family_index, VkQueue queue, VkBuffer src_buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandPool command_pool = IVRSingleCommandUtil::CreateCommandPool(logical_device, queue_family_index);
		VkCommandBuffer command_buffer = IVRSingleCommandUtil::BeginSingleTimeCommands(logical_device, command_pool);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(command_buffer, src_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		IVRSingleCommandUtil::SubmitAndEndSingleTimeCommands(logical_device, queue, command_buffer, command_pool);
	}

	static void TransitionImageLayout(VkDevice logical_device, uint32_t queue_family_index, VkQueue queue,
		VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
        VkCommandPool command_pool = IVRSingleCommandUtil::CreateCommandPool(logical_device, queue_family_index);
        VkCommandBuffer command_buffer = IVRSingleCommandUtil::BeginSingleTimeCommands(logical_device, command_pool);

        //a common way to perform layout transitions is using an image memory barrier
        //this kind of pipeline barrier like this is generally used to synchronize access to resources
        // like ensuring that a write to buffer is complete before reading from it
        //But it can also be used to transition image layouts and transfer queue family ownership
        // when VK_SHARING_MODE_EXCLUSIVE is used
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //ignored because we dont want to transfer queue family ownership
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        // Note : interestingly imageviewcreateinfo also have subresourceRange with the same fields
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (HasStencilComponent(format))
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        //Note : In PipelineManager SubpassDependency have the same following two fields
        barrier.srcAccessMask = 0; //To Do
        barrier.dstAccessMask = 0; //To Do

        //There are two transitions that we need to handle
        // 1. Transitioning from undefined to transfer destination
        // 2. Transitioning from transfer destination to shader read : 
        //      shader read should wait on transfer write (specifically the shader read in fragment shader)

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            //depth buffer will be read to perform depth tests to see if a fragment is visible
            //  and it will be written to when a new fragment is drawn
            //reading happens in VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
            //writing happens in VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
        }
        else
        {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(command_buffer,
            source_stage,
            destination_stage,
            0, 0,
            nullptr, 0, nullptr, 1, &barrier);

        IVRSingleCommandUtil::SubmitAndEndSingleTimeCommands(logical_device, queue, command_buffer, command_pool);
	}

    static void CreateVkImageAndBindMemory(VkDevice logical_device, VkPhysicalDevice physical_device,
        uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags,
        VkImage& image, VkDeviceMemory& image_memory)
    {
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;

        image_info.format = format; //need to use the same format for texels as the pixels in the buffer
        image_info.tiling = tiling; //texels are laid out in an implementation defined order for optimal access from the shader

        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //there are two possible initial layout values:
        // 1. undefined (like above) - not usable by GPU, first layout transition will discard the texels
        // 2. preinitialized - not usable by GPU, but first layout transition will preserve the texels
        // in our case, we will transition the image to be a transfer destination and then copy texel data to it from a buffer object

        image_info.usage = usage;
        //sampled_bit because we want to access the image from the shader 

        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //image will be used by a single queue family

        image_info.samples = VK_SAMPLE_COUNT_1_BIT; //related to multisampling, only relevant for images that will be used as attachments
        image_info.flags = 0; //some optional flags for images that will be used as sparse images

        if (vkCreateImage(logical_device, &image_info, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create texture image");
        }

        
        //After creating the image itself, we also need to bind memory for the image to use
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = IVRBufferUtilities::FindMemoryType(physical_device,
            memory_requirements.memoryTypeBits, memory_property_flags);

        if (vkAllocateMemory(logical_device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory");
        }

        vkBindImageMemory(logical_device, image, image_memory, 0);
    }

    static bool HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

};
