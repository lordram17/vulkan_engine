#include "camera.h"

void IVRCamera::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding;
    ubo_layout_binding.binding = 0; //specifies the binding used in the shader
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //it is possible for the shader variable to represent an array of uniform buffers
    ubo_layout_binding.descriptorCount = 1; //we have a single uniform buffer

    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //specify which shader stages the descriptor is going to be referenced
    //this can be VK_SHADER_STAGE_ALL_GRAPHICS or a combination of VkShaderStageFlagBits values

    ubo_layout_binding.pImmutableSamplers = nullptr; //only relevant to image sampling related descriptors
    
}