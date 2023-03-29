#version 450

//the following binding will be referenced in the descriptor layout
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location=0) in vec2 inPosition; //dvec3 uses 2 slots, so the location of inColor must be 2 higher
layout(location=1) in vec3 inColor;
layout(location=2) in vec2 inTexCoord;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_coord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    frag_color = inColor;
    frag_tex_coord = inTexCoord;
}