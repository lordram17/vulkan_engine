#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 frag_position;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_tex_coord;

layout(binding=1) uniform LightUniformBufferObject {
    vec3 position;
    vec3 direction;
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
} dir_light;

layout(binding=2) uniform MaterialUniformBufferObject {
    float specular_power;
    int is_cube_map;
    vec3 specular_color;
    vec3 diffuse_color;
} material;

layout(binding = 3) uniform sampler2D tex_sampler;

void main() {

    vec3 halfway_vector = (-normalize(dir_light.direction) + -normalize(frag_position)) / (length(-normalize(dir_light.direction) + -normalize(frag_position)));

    float diffuse_intensity = max(dot(normalize(frag_normal), -normalize(dir_light.direction)), 0.0);
    vec3 diffuse = diffuse_intensity * (material.diffuse_color * dir_light.diffuse_color);

    float specular_intensity = pow(max(dot(normalize(frag_normal), halfway_vector), 0.0), material.specular_power);
    vec3 specular = specular_intensity * (material.specular_color * dir_light.specular_color);

    vec3 ambient = dir_light.ambient_color;

    vec3 texture_color = texture(tex_sampler, frag_tex_coord).rgb;

    outColor = vec4((ambient + diffuse + specular) * texture_color, 1.0);

}