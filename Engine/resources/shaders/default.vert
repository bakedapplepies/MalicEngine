#version 450

// Vertex Data
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

// Fragment shader input
layout(location = 0) out vec4 out_position;
layout(location = 1) out vec3 out_color;

// Uniforms
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} u_mvp;

void main()
{
    gl_Position = u_mvp.projection * u_mvp.view * u_mvp.model * vec4(in_position, 1.0);
    out_color = in_color;
}
