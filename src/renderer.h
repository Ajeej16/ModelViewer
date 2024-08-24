
#ifndef RENDERER_H
#define RENDERER_H

enum {
    PRIMITIVE_POINTS = 0,
    PRIMITIVE_LINES,
    PRIMITIVE_LINE_LOOP,
    PRIMITIVE_LINE_STRIP,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP,
    PRIMITIVE_TRIANGLE_FAN,
    PRIMITIVE_QUADS,
};

typedef enum vbo_type_t {
    VBO_VERTS = 0,
    VBO_UVS,
    VBO_COLORS,
    VBO_NORMS,
    VBO_INDICES,
    VBO_COUNT,
} vbo_type_t;

typedef struct color_t {
    u8 r, g, b, a;
} color_t;

typedef struct mesh_info_t {
    u32 mesh_id;
    u32 vert_idx;
    u32 indices_idx;
    u32 indices_count;
} mesh_info_t;

typedef struct model_info_t {
    mesh_info_t *meshes;
    u32 mesh_count;
    
    u32 trans_id;
} model_info_t;

typedef struct entity_t {
    model_info_t m_info;
    
    v3 pos;
    v3 rot;
    v3 scale;
} entity_t;

typedef struct render_cmd_t {
    u32 mesh_id;
    u32 trans_id;
    
    u32 vert_idx;
    u32 indices_idx;
    u32 indices_count;
    
    u32 prim_type;
} render_cmd_t;

typedef struct camera_t {
    v3 pos;
    v3 rot;
    v3 front, up, right;
    v3 world_up;
    
    f32 speed;
    f32 sens;
    f32 fov;
    f32 aspect_ratio;
    f32 near;
    f32 far;
    
    mat4 proj;
} camera_t;

typedef struct light_t {
    v3 pos;
    f32 constant;
    f32 linear;
    f32 quadratic;
} light_t;

typedef struct framebuffer_t {
    u32 id, rbo;
    u32 tex;
    u32 shader;
    u32 vao;
} framebuffer_t;

typedef struct gl_renderer {
    STACK(v3) *verts;
    STACK(v2) *uvs;
    STACK(color_t) *colors;
    STACK(v3) *norms;
    STACK(u32) *indices;
    
    STACK(render_cmd_t) *cmds;
    STACK(mat4) *transforms;
    STACK(entity_t) *entities;
    
    u32 vbos[VBO_COUNT], vao;
    STACK(u32) *shaders;
    
    light_t lights[6];
    u32 light_count;
    
    framebuffer_t fb;
    
    u32 gpu_transform;
    u32 use_textures;
    
    camera_t *camera;
} gl_renderer;

#define COLOR(r, g, b, a) color_t{ r, g, b, a }

internal u32
get_shader(gl_renderer *gl, u32 id) {
    return gl->shaders[id];
}

#endif //RENDERER_H
