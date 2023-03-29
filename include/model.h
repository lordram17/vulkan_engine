#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <iostream>
#include <cstring>
#include "buffer_utils.h"


struct Vertex {
    //vertex has 2 attributes
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    
    //Vulkan needs 2 pieces of information to determine how to pass data from GPU memory
    // to the vertex shader

    // 1. Binding Description 
    // A binding is between a command buffer and vertex buffer using the command vkCmdBindVertexBuffers
    // Binding description describes at which rate to load data from memory throughout the vertices
    // Specifies the number of bytes between data entries and whether to move to the next data entry
    //  after each vertex or after each instance
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; //binding index in the array of bindings (this is used at many places)
        bindingDescription.stride = sizeof(Vertex); //number of bytes from one entry to next
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //move to next data entry after each vertex

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescription ()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

        //the first attribute is position (corresponding to location=0 in the vertex shader)
        attributeDescriptions[0].binding = 0; //from which binding index does the per vertex data come?
        attributeDescriptions[0].location = 0; //location directive of the input in vertex shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //type of data of this attribute
        //the number of color channelsbuffer_spawner
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    } 
};


class IVRModel {

private:
    IVRModel(const IVRModel&) = delete;

    VkDeviceMemory VertexBufferMemory_;
    uint32_t VertexCount_;
    VkDeviceMemory IndexBufferMemory_;

    VkDevice LogicalDevice_;
    VkPhysicalDevice PhysicalDevice_;
    VkQueue Queue_;
    uint32_t QueueFamilyIndex_;

public:
    IVRModel(VkDevice logical_device, VkPhysicalDevice physical_device, 
        VkQueue queue, uint32_t queue_family_index);
    ~IVRModel();

    VkBuffer VertexBuffer_;
    VkBuffer IndexBuffer_;

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };

    void CreateVertexBuffer();
    void CreateIndexBuffer();

    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

    
};
