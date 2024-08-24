
#ifndef ASSETS_H
#define ASSETS_H

typedef struct mesh_t {
    v3 *verts;
    v2 *uvs;
    color_t *colors;
    v3 *norms;
    u32 *indices;
    
    u32 vert_count;
    u32 indices_count;
    
    u32 mat_id;
} mesh_t;

// TODO(ajeej): flesh out when implementing materials
typedef struct material_t {
    f32 ambient[3];
    f32 diffuse[3];
    f32 specular[3];
    f32 transmittance[3];
    f32 emission[3];
    f32 shininess;
    f32 index_of_refraction;
    f32 dissolve;
    i32 illum;
    
    union {
        struct {
            u32 ambient_tex_id;
            u32 diffuse_tex_id;
            u32 specular_tex_id;
            u32 specular_highlight_tex_id;
            u32 bump_tex_id;
            u32 displacement_tex_id;
            u32 alpha_tex_id;
        };
        
        u32 tex_ids[7];
    };
} material_t;

typedef struct texture_t {
    u8 *data;
    u32 w, h;
    u32 bytes_per_pixel;
    u32 id;
} texture_t;

typedef struct asset_manager_t {
    STACK(mesh_t) *meshes;
    STACK(material_t) *mats;
    STACK(texture_t) *textures;
} asset_manager_t;

#endif //ASSETS_H
