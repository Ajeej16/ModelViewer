#version 330 core

out vec4 frag_color;

in vec3 frag_pos;
in vec3 out_norm;
in vec2 out_uv;
in vec4 out_color;

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

vec3 calculate_phong(point_light_t light, vec3 N, vec3 P) {
    vec3 light_vec = light.pos - P;
    vec3 V = normalize(-P);
    vec3 L = normalize(light_vec);
    vec3 R = reflect(-L, N);
    
    vec3 ambient = mat.ambient * vec3(texture(mat.diffuse_tex, out_uv));
    vec3 diffuse = max(dot(N, L), 0.0)*mat.diffuse * vec3(texture(mat.diffuse_tex, out_uv));
    vec3 specular = pow(max(dot(R, V), 0.0), mat.shininess)*mat.specular * vec3(texture(mat.specular_tex, out_uv));
    
    float distance = length(light_vec);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    return (ambient + diffuse + specular)*attenuation*2;
}

void main() {
    vec3 color = vec3(0.0);
    
    for (int i = 0; i < point_light_count; i++)
        color += calculate_phong(point_lights[i], out_norm, frag_pos);
    
    frag_color = vec4(color, 1.0) * out_color;
}