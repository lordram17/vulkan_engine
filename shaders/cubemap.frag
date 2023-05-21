#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 cube_tex_coord;

layout(binding = 1) uniform samplerCube cubemap;

void main() {
    outColor = texture(cubemap, cube_tex_coord);
}