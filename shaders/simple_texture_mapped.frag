#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 frag_position;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_tex_coord;
layout(location = 3) in vec3 camera_world_pos;
layout(location = 4) in vec4 light_space_pos;

layout(binding=1) uniform LightUniformBufferObject {
    vec3 position;
    vec3 direction;
    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
} dir_light;

layout(binding=4) uniform MaterialUniformBufferObject {
    float specular_power;
    int is_cube_map;
    vec3 specular_color;
    vec3 diffuse_color;
} material;

layout(binding = 3) uniform sampler2D depth_tex_sampler;
layout(binding = 5) uniform sampler2D tex_sampler;

void main() {
    vec3 view_direction = camera_world_pos - frag_position;

    vec3 halfway_vector = normalize(-normalize(dir_light.direction) + normalize(view_direction));

    float diffuse_intensity = max(dot(normalize(frag_normal), -normalize(dir_light.direction)), 0.0);
    vec3 diffuse = diffuse_intensity * (material.diffuse_color * dir_light.diffuse_color);

    float specular_intensity = pow(max(dot(normalize(frag_normal), halfway_vector), 0.0), material.specular_power);

    vec3 specular = specular_intensity * (material.specular_color * dir_light.specular_color);

    vec3 ambient = dir_light.ambient_color;

    vec3 texture_color = texture(tex_sampler, frag_tex_coord).rgb;

    vec4 shadow_coord = light_space_pos / light_space_pos.w;

    float x = shadow_coord.x * 0.5 + 0.5;
    float y = shadow_coord.y * 0.5 + 0.5;
    vec2 depth_tex_sample_coord = vec2(x, y);
    float light_depth = texture(depth_tex_sampler, depth_tex_sample_coord).r;

    if(shadow_coord.z - 0.0005 > light_depth) 
    { 
        diffuse *= 0.5;
        specular *= 0.5;
    }

    outColor = vec4((ambient + diffuse + specular) * texture_color, 1.0);

}