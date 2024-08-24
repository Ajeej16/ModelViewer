#version 330 core

out vec4 frag_color;

in vec2 out_uv;

uniform sampler2D tex;

void main() {
    vec3 color = texture(tex, out_uv).rgb;
    frag_color = vec4(color, 1.0);
}