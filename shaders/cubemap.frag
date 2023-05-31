#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 cube_tex_coord;

layout(binding=1) uniform LightUniformBufferObject {
    vec3 position;
    vec3 direction;
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
} dir_light;

layout(binding=3) uniform sampler2D diffuse_tex_sampler;

layout(binding=4) uniform MaterialUniformBufferObject {
    vec3 specular_color;
    float specular_power;
    vec3 diffuse_color;
    bool is_cube_map;
} material;

layout(binding = 5) uniform samplerCube cubemap;

void main() {
    outColor = texture(cubemap, cube_tex_coord);
}