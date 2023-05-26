#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

IVRModel::IVRModel(std::shared_ptr<IVRDeviceManager> device_manager, std::string model_name, std::string model_path) :
    DeviceManager_{ device_manager }, Name_{ model_name }
{
    ModelPath_ = IVRPath::GetCrossPlatformPath({ "3d_models", model_path});
    LoadModel();
    CreateVertexBuffer();
    CreateIndexBuffer();
}

IVRModel::~IVRModel()
{
    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), VertexBuffer_, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), VertexBufferMemory_, nullptr);

    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), IndexBuffer_, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), IndexBufferMemory_, nullptr);
}

void IVRModel::LoadModel()
{
    //an obj file consists of positions, normals, texture coordinates and faces
    //faces consist of an arbitrary number of vertices, where each vertex refers to a position,
    //   normal and texture coordinate

    // tinyobj::attrib_t container holders positions, normals and texture coordinates in
    //   attrib.vertices, attrib.normals and attrib.texcoords vectors respectively

    //shape container contains all of the separate objects and their faces
    //   each face consists of an array of vertices, and each vertex contains the indices of the position, normal and texture coordinate attributes
    //  obj models can also define a material and texture per face, but we will be ignoring those

    //err string contains errors and warn string contains warnings that ocurred while loading the file

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::cout << "Opening model : " << ModelPath_ << "\n";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ModelPath_.c_str())) {
        throw std::runtime_error(warn + err);
    }

    for(const tinyobj::shape_t shape : shapes)
    {
        for(const tinyobj::index_t index : shape.mesh.indices)
        {
            Vertex vertex{};

            //need to use the index to query the actual vertices and texture coordinates
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            Vertices.push_back(vertex);
            Indices.push_back(Indices.size()); 
        }
    }
}

void IVRModel::CreateVertexBuffer()
{
    VkDeviceSize buffer_size = sizeof(Vertices[0]) * Vertices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    IVRBufferUtilities::Spawn(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, //buffer can be used as source in memory transfer operation
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_buffer_memory);

    //TRANSFER vertex data from host memory to staging buffer memory
    void* data;
    vkMapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, Vertices.data(), (size_t)buffer_size);
    vkUnmapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory);

    IVRBufferUtilities::Spawn(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        buffer_size, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VertexBuffer_, VertexBufferMemory_);

    IVRBufferUtilities::TransferBufferData(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        staging_buffer, VertexBuffer_, buffer_size);
    
    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), staging_buffer, nullptr);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, nullptr);
}

void IVRModel::CreateIndexBuffer()
{
    VkDeviceSize buffer_size = sizeof(Indices[0]) * Indices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    IVRBufferUtilities::Spawn(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_buffer_memory);
    
    void* data;
    vkMapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, Indices.data(), (size_t)buffer_size);
    vkUnmapMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory);

    IVRBufferUtilities::Spawn(DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(),
        buffer_size, 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        IndexBuffer_, IndexBufferMemory_);
    
    IVRBufferUtilities::TransferBufferData(
        DeviceManager_->GetLogicalDevice(), DeviceManager_->GetPhysicalDevice(), DeviceManager_->GetDeviceQueueFamilies().graphicsFamily, DeviceManager_->GetGraphicsQueue(),
        staging_buffer, IndexBuffer_, buffer_size);
    
    vkDestroyBuffer(DeviceManager_->GetLogicalDevice(), staging_buffer, VK_NULL_HANDLE);
    vkFreeMemory(DeviceManager_->GetLogicalDevice(), staging_buffer_memory, VK_NULL_HANDLE);

}

uint32_t IVRModel::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    //graphic cards can offer different types of memory to allocate from
    //each memory varies in terms of allowed operations and performance characteristics
    //we need to find the right type of memory to use

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(DeviceManager_->GetPhysicalDevice(), &memory_properties);

    //memory_properties has two arrays : memoryTypes and memoryHeaps
    // memoryHeaps are distinct memory resources like dedicated VRAM and swap space in RAM (if VRAM runs out)
    // right now we are only concerned with memoryType and not the heap where it comes from

    for(uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        //dont completely understand this if statement yet
        if((type_filter & (1 << i)) && 
        (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0;
}

IVRTransform IVRModel::GetTransform()
{
    return Transform_;
}

void IVRModel::SetPosition(glm::vec3 position)
{
	Transform_.Position = position;
}

void IVRModel::SetRotation(glm::vec3 rotation)
{
	Transform_.Rotation = rotation;
}

void IVRModel::SetScale(glm::vec3 scale)
{
	Transform_.Scale = scale;
}

VkBuffer IVRModel::GetVertexBuffer()
{
	return VertexBuffer_;
}

VkBuffer IVRModel::GetIndexBuffer()
{
	return IndexBuffer_;
}