#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 norm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 out_color;
out vec2 out_uv;

void main() {
    gl_Position = proj*view*model*vec4(pos, 1.0);
    out_color = color;
    out_uv = uv;
}