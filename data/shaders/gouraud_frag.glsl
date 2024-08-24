#version 330 core

out vec4 frag_color;

in vec3 out_color;
in vec2 out_uv;

void main() {
    frag_color = vec4(out_color, 1.0);
}