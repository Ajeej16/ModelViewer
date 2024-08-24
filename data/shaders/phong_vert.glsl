#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 norm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 frag_pos;
out vec3 out_norm;
out vec2 out_uv;
out vec4 out_color;

void main() {
    frag_pos = vec3(model * vec4(pos, 1.0));
    out_norm = normalize(mat3(transpose(inverse(model)))*norm);
    out_uv = uv;
    out_color = color;
    
    gl_Position = proj*view*vec4(frag_pos, 1.0);
}