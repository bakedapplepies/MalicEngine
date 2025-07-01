#version 450

layout(location = 0) out vec4 OutColor;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;

layout(binding = 1) uniform sampler2D u_sampler;

void main()
{
    OutColor = vec4(texture(u_sampler, in_uv).rgb, 1.0);
}