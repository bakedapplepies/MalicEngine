#version 450

// Note: gl_VertexIndex is only available when targeting Vulkan

// vec2 positions[3] = {
//     vec2( 0.0, -0.5),
//     vec2( 0.5,  0.5),
//     vec2(-0.5,  0.5),
// };

// vec3 colors[3] = {
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// };

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec3 out_color;

void main()
{
    gl_Position = in_position;
    out_color = in_color;
}
