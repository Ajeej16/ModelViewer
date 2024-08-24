
#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

typedef union vertex_t {
    struct {
        f32 v[3];
        f32 vn[3];
        f32 vt[2];
    };
    
    f32 el[8];
} vertex_t;

typedef struct obj_mat_t {
    char *name;
    
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
            char *ambient_texture_name;
            char *diffuse_texture_name;
            char *specular_texture_name;
            char *specular_highlight_texture_name;
            char *bump_texture_name;
            char *displacement_texture_name;
            char *alpha_texture_name;
        };
        
        char *tex_names[7];
    };
} obj_mat_t;

typedef struct obj_t {
    char *name;
    char *mat_name;
    
    u32 idx_offset;
    u32 idx_count;
    u32 mat_id;
} obj_t;

typedef union vert_idx_t {
    struct {
        i32 v, vn, vt;
    };
    
    i32 el[3];
} vert_idx_t;

typedef struct obj_info_t {
    char *path;
    
    STACK(obj_t) *objs;
    STACK(obj_mat_t) *materials;
    
    STACK(f32) *v;
    STACK(f32) *vn;
    STACK(f32) *vt;
    
    union {
        STACK(i32) *idx;
        STACK(vert_idx_t) *v_idx;
    };
} obj_info_t;

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

#endif //OBJ_LOADER_H
