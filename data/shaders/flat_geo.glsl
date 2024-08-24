#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

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

in vec3 geo_pos[];
in vec4 geo_color[];

out vec3 out_color;

vec3 calculate_norm() {
    vec3 a = geo_pos[0] - geo_pos[1];
    vec3 b = geo_pos[2] - geo_pos[1];
    
    return normalize(cross(b, a));
}

vec3 calculate_flat(point_light_t light, vec3 N, vec3 P) {
    vec3 light_vec = light.pos - P;
    vec3 L = normalize(light_vec);
    vec3 V = normalize(-P.xyz);
    vec3 R = reflect(-L, N);
    
    float distance = length(light_vec);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    vec3 diffuse = max(dot(N, L), 0.0) * mat.diffuse;
    vec3 specular = pow(max(dot(R, V), 0.0), mat.shininess) * mat.specular;
    
    return (mat.ambient + diffuse + specular) * attenuation * 3;
}

void main() {
    out_color = vec3(0.0);
    
    vec3 P = (geo_pos[0] + geo_pos[1] + geo_pos[2])/3.0;
    vec3 N = calculate_norm();
    for (int i = 0; i < point_light_count; i++)
        out_color += calculate_flat(point_lights[i], N, P);
    
    out_color *= vec3(geo_color[0]);
    
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
}