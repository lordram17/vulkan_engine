#version 450

//the following binding will be referenced in the descriptor layout
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding=1) uniform LightUniformBufferObject {
    vec3 position;
    vec3 direction;
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
} dir_light;

layout(location=0) in vec3 inPosition; //dvec3 uses 2 slots, so the location of inColor must be 2 higher
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inTexCoord;

layout(location = 0) out vec3 frag_position;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec2 frag_tex_coord;
layout(location = 3) out vec3 camera_world_pos;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    
    frag_position = (ubo.model * vec4(inPosition, 1.0)).xyz;
    frag_normal = (ubo.model * vec4(inNormal, 0.0)).xyz;
    frag_tex_coord = inTexCoord;

    camera_world_pos = (inverse(ubo.view)[3]).xyz;
}