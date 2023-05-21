#version 450

//the following binding will be referenced in the descriptor layout
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location=0) in vec3 inPosition; //dvec3 uses 2 slots, so the location of inColor must be 2 higher
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inTexCoord;

layout(location = 0) out vec3 cube_tex_coord;

void main() {

    vec3 position = mat3(ubo.view * ubo.model) * inPosition.xyz; //remove translation from the view matrix
    gl_Position = (ubo.proj * vec4(position, 1.0)).xyzz;
    cube_tex_coord = inPosition; //the texture coordinate for the cube is the direction from the center of the cube to the vertex (this does not have to be normalized)
}