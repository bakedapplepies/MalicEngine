#version 450

layout(location = 0) out vec4 OutColor;

layout(location = 0) in vec3 in_color;

void main()
{
    OutColor = vec4(in_color, 1.0);
}