#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 norm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

struct point_light_t {
    vec3 pos;
    float constant;
    float linear;
    float quadratic;
};

struct material_t {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    
    sampler2D diffuse_tex;
    sampler2D specular_tex;
};

#define MAX_LIGHTS 6
uniform point_light_t point_lights[MAX_LIGHTS];
uniform int point_light_count;

uniform material_t mat;

out vec3 out_color;
out vec2 out_uv;

vec3 calculate_gouraud(point_light_t light, vec3 N, vec3 P) {
    vec3 light_vec = light.pos - P;
    vec3 L = normalize(light_vec);
    vec3 V = normalize(-P);
    vec3 R = reflect(-L, N);
    
    float distance = length(light_vec);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    vec3 diffuse = max(dot(N, L), 0.0) * mat.diffuse;
    vec3 specular = pow(max(dot(R, V), 0.0), mat.shininess) * mat.specular;
    
    return (mat.ambient + diffuse + specular) * attenuation * 2;
}

void main() {
    out_color = vec3(0.0);
    
    vec3 P = vec3(model * vec4(pos, 1.0));
    vec3 N = normalize(mat3(inverse(transpose(model)))*norm);
    for (int i = 0; i < point_light_count; i++)
        out_color += calculate_gouraud(point_lights[i], N, P);
    
    out_color *= vec3(color);
    
    gl_Position = proj*view*vec4(P, 1.0);
}