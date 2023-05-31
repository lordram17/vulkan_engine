#version 450

//the following binding will be referenced in the descriptor layout
layout(binding = 0) uniform LightMVPUbo{
    mat4 model;
    mat4 view;
    mat4 proj;
} light_mvp;

layout(location=0) in vec3 inPosition; //dvec3 uses 2 slots, so the location of inColor must be 2 higher
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inTexCoord;

layout(location=0) out vec4 frag_pos;



void main() {
    gl_Position = light_mvp.proj * light_mvp.view * light_mvp.model * vec4(inPosition, 1.0);
    frag_pos = gl_Position;
}