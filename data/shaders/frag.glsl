#version 330 core

out vec4 frag_color;

in vec4 out_color;
in vec2 out_uv;

struct material_t {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    
    sampler2D diffuse_tex;
    sampler2D specular_tex;
};

uniform material_t mat;

void main() {
    frag_color = out_color;//vec4(vec3(texture(mat.diffuse_tex, out_uv))*mat.diffuse, 1.0)*out_color;
}