
internal u32
create_texture_from_bitmap(asset_manager_t *am, u8 *data, i32 w, i32 h, i32 ch) {
    u32 tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    switch(ch) {
        case 1: {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED,
                         GL_UNSIGNED_BYTE, data);
        } break;
        
        case 2: {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, w, h, 0, GL_RG,
                         GL_UNSIGNED_BYTE, data);
        } break;
        
        case 3: {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, data);
        } break;
        
        case 4: {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, data);
        } break;
        
        default: { ASSERT(0, "Unsupported Texture format."); } break;
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    u32 idx = get_stack_count(am->textures);
    texture_t *tex = (texture_t *)stack_push(&am->textures);
    tex->data = (u8 *)malloc(ch*w*h);
    memcpy(tex->data, data, ch*w*h);
    tex->w = (u32)w;
    tex->h = (u32)h;
    tex->bytes_per_pixel = ch;
    tex->id = tex_id;
    
    return idx;
}

internal void
init_asset_manager(asset_manager_t *am)
{
    am->meshes = NULL;
    am->mats = NULL;
    am->textures = NULL;
    
    u32 white = 0xFFFFFFFF;
    create_texture_from_bitmap(am, (u8 *)&white, 1, 1, 4);
    
    material_t *mat = (material_t *)stack_push(&am->mats);
    memset(mat, 0, sizeof(mat));
    u32 i;
    for (i = 0; i < 3; i++) 
        mat->ambient[i] = 0.8f;
    for (i = 0; i < 3; i++)
        mat->diffuse[i] = 0.8f;
    for (i = 0; i < 3; i++)
        mat->specular[i] = 0.8f;
    mat->shininess = 500;
}

internal void
init_mesh(mesh_t *mesh,
          u32 v_count, u32 i_count)
{
    u64 v_size = sizeof(v3)*v_count;
    u64 uv_size = sizeof(v2)*v_count;
    u64 c_size = sizeof(color_t)*v_count;
    u64 n_size = sizeof(v3)*v_count;
    u64 i_size = sizeof(u32)*i_count;
    
    u8 *block = (u8 *)malloc(v_size+uv_size+c_size+
                             n_size+i_size);
    
    mesh->verts = (v3 *)block;
    block += v_size;
    mesh->uvs = (v2 *)block;
    block += uv_size;
    mesh->colors = (color_t *)block;
    block += c_size;
    mesh->norms = (v3 *)block;
    block += n_size;
    mesh->indices = (u32 *)block;
    
    mesh->vert_count = v_count;
    mesh->indices_count = i_count;
    
    mesh->mat_id = 0;
}

internal inline material_t *
get_mesh_material(asset_manager_t *am, u32 mesh_id)
{
    return am->mats+am->meshes[mesh_id].mat_id;
}

internal u64
hash_vert_idx(i32 *indices) {
    u64 hash = 0;
    
    for (u32 i = 0; i < 3; i++) {
        u64 value;
        memcpy(&value, &indices[i], sizeof(i32));
        hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    
    return hash;
}

#define kh_key_t vertex_t
#define kh_val_t u32
#define kh_hash_func(a) (hash_vert_idx(a.el))
#define kh_hash_equal(a, b) (a.v == b.v && a.vn == b.vn && a.vt == b.vt)
#include "khash.h"
KHASH_INIT(vertex_map_t, vert_idx_t, u32, 1, kh_hash_func, kh_hash_equal)

internal void
create_model(mesh_t *meshes, obj_info_t *info, u32 mat_idx, color_t color = color_t{ 0, 0, 0, 255 })
{
    f32 *sv, *sn, *st;
    v3 *verts = NULL, *v; v3 *norms = NULL, *n; v2 *uvs = NULL, *t;
    u32 *indices = NULL;
    vertex_t vert;
    
    khash_t(vertex_map_t) *vertex_map = kh_init(vertex_map_t);;
    
    obj_t obj; mesh_t *mesh;
    u32 obj_count = get_stack_count(info->objs);
    
    u32 cur_idx = 0;
    
    for(u32 o_idx = 0; o_idx < obj_count; o_idx++) {
        obj = info->objs[o_idx];
        mesh = meshes+o_idx;
        
        vert_idx_t *idx = info->v_idx+obj.idx_offset;
        for(u32 i_idx = 0; i_idx < obj.idx_count; i_idx++, idx++) {
            
            khiter_t it = kh_get(vertex_map_t, vertex_map, *idx);
            u32 *i = (u32 *)stack_push(&indices);
            if (it != kh_end(vertex_map)) {
                *i = kh_value(vertex_map, it);
            } else {
                i32 value;
                khint_t k = kh_put(vertex_map_t, vertex_map, *idx, (int *)&value);
                if(!value) kh_del(vertex_map_t, vertex_map, k);
                
                kh_value(vertex_map, k) = cur_idx;
                *i = cur_idx++;
                
                sv = info->v+idx->v*3;
                v = (v3 *)stack_push(&verts);
                *v = HMM_V3(sv[0], sv[1], sv[2]);
                
                if(idx->vn != (u32)(-1)) {
                    sn = info->vn+idx->vn*3;
                    n = (v3 *)stack_push(&norms);
                    *n = HMM_V3(sn[0], sn[1], sn[2]);
                }
                
                if(idx->vt != (u32)(-1)) {
                    st = info->vt+idx->vt*2;
                    t = (v2 *)stack_push(&uvs);
                    *t = HMM_V2(st[0], st[1]);
                }
            }
            
        }
        
        u32 v_count = get_stack_count(verts);
        u32 i_count = get_stack_count(indices);
        
        //for(u32 i = 0; i < i_count; i++)
        //LOG("%u", indices[i]);
        
        mesh->verts = (v3 *)malloc(sizeof(v3)*v_count);
        mesh->uvs = (v2 *)malloc(sizeof(v2)*v_count);
        memset(mesh->uvs, 0, sizeof(v2)*v_count);
        mesh->norms = (v3 *)malloc(sizeof(v3)*v_count);
        memset(mesh->norms, 0, sizeof(v3)*v_count);
        mesh->colors = (color_t *)malloc(sizeof(color_t)*v_count);
        mesh->indices = (u32 *)malloc(sizeof(u32)*i_count);
        
        memcpy(mesh->verts, verts, sizeof(v3)*v_count);
        
        if(uvs)
            memcpy(mesh->uvs, uvs, sizeof(v2)*v_count);
        if(norms)
            memcpy(mesh->norms, norms, sizeof(v3)*v_count);
        
        for(u32 i = 0; i < v_count; i ++)
            mesh->colors[i] = color;
        
        memcpy(mesh->indices, indices, sizeof(u32)*i_count);
        
        mesh->vert_count = v_count;
        mesh->indices_count = i_count;
        
        mesh->mat_id = obj.mat_id + mat_idx;
        
        stack_clear(verts);
        stack_clear(uvs);
        stack_clear(norms);
        stack_clear(indices);
        
    }
    kh_destroy(vertex_map_t, vertex_map);
    
    stack_free(verts);
    if(uvs)
        stack_free(uvs);
    if(norms)
        stack_free(norms);
    stack_free(indices);
}

internal u32
create_texture(asset_manager_t *am, char *filename)
{
    i32 w, h, ch;
    u8 *data = (u8 *)stbi_load(filename, &w, &h, &ch, 0);
    
    int id = create_texture_from_bitmap(am, data, w, h, ch);
    
    stbi_image_free(data);
    
    return id;
}

internal void
create_materials(asset_manager_t *am, material_t *mats, obj_info_t *info)
{
    material_t *mat;
    obj_mat_t obj_mat;
    
    char path[PATH_MAX];
    strcpy(path, info->path);
    char *last_slash = strrchr(path, '\\');
    
    u32 mat_count = get_stack_count(info->materials);
    for(u32 m_idx = 0; m_idx < mat_count; m_idx++) {
        mat = mats + m_idx;
        obj_mat = info->materials[m_idx];
        
        memcpy(mat->ambient, obj_mat.ambient, sizeof(f32)*3);
        memcpy(mat->diffuse, obj_mat.diffuse, sizeof(f32)*3);
        memcpy(mat->specular, obj_mat.specular, sizeof(f32)*3);
        memcpy(mat->transmittance, obj_mat.transmittance, sizeof(f32)*3);
        memcpy(mat->emission, obj_mat.emission, sizeof(f32)*3);
        mat->shininess = obj_mat.shininess;
        mat->index_of_refraction = obj_mat.index_of_refraction;
        mat->dissolve = obj_mat.dissolve;
        mat->illum = obj_mat.illum;
        
        // TODO(ajeej): remove double back slashes
        char **tex_name = obj_mat.tex_names;
        u32 *tex_id = mat->tex_ids;
        for(u32 t_idx = 0; t_idx < 7; t_idx++, tex_name++, tex_id++) {
            if(*tex_name) {
                strcpy(last_slash+1, *tex_name);
                *tex_id = create_texture(am, path);
            } else {
                *tex_id = 0;
            }
        }
    }
}

internal void
add_model(asset_manager_t *am, char *filename, model_info_t *m_info,
          color_t color = color_t{ 0, 0, 0, 255 })
{
    obj_info_t info = load_obj(filename);
    
    u32 mesh_count = get_stack_count(info.objs);
    u32 mat_count = get_stack_count(info.materials);
    
    u32 mesh_idx = get_stack_count(am->meshes);
    u32 mat_idx = get_stack_count(am->mats);
    
    if(mesh_count) {
        mesh_t *meshes = (mesh_t *)stack_push_array(&am->meshes, mesh_count);
        create_model(meshes, &info, mat_idx, color);
        
        if(mat_count) {
            material_t *mats = (material_t *)stack_push_array(&am->mats, mat_count);
            create_materials(am, mats, &info);
        }
        else 
        {
            for (u32 i = 0; i < mesh_count; i++)
                meshes[i].mat_id = 0;
        }
    }
    
    free_obj(&info);
    
    mesh_info_t *mesh_infos = (mesh_info_t *)malloc(sizeof(mesh_info_t)*mesh_count);
    
    for(u32 i = 0; i < mesh_count; i++)
        mesh_infos[i].mesh_id = mesh_idx+i;
    
    m_info->meshes = mesh_infos;
    m_info->mesh_count = mesh_count;
}

internal void
create_plane(mesh_t *mesh, f32 w, f32 h, color_t color)
{
    u32 i;
    f32 h_w = w*0.5f;
    f32 h_h = h*0.5f;
    
    u32 n_i[] = { 0, 1, 2, 2, 3, 0 };
    
    v3 *verts = mesh->verts;
    verts[0] = HMM_V3(-h_w, 0.0f, -h_h);
    verts[1] = HMM_V3(h_w, 0.0f, -h_h);
    verts[2] = HMM_V3(h_w, 0.0f, h_h);
    verts[3] = HMM_V3(-h_w, 0.0f, h_h);
    
    v2 *uvs = mesh->uvs;
    uvs[0] = HMM_V2(0.0f, 0.0f);
    uvs[1] = HMM_V2(1.0f, 0.0f);
    uvs[2] = HMM_V2(1.0f, 1.0f);
    uvs[3] = HMM_V2(0.0f, 1.0f);
    
    color_t *colors = mesh->colors;
    for(i = 0; i < 4; i++)
        colors[i] = color;
    
    v3 *norms = mesh->norms;
    for(i = 0; i < 4; i++)
        norms[i] = HMM_V3(0.0f, 1.0f, 0.0f);
    
    u32 *indices = mesh->indices;
    memcpy(indices, n_i, sizeof(n_i));
}

internal void
add_plane(asset_manager_t *am, f32 w, f32 h, color_t color, model_info_t *info)
{
    u32 id = get_stack_count(am->meshes);
    mesh_t *mesh = (mesh_t *)stack_push(&am->meshes);
    
    init_mesh(mesh, 4, 6);
    create_plane(mesh, w, h, color);
    
    info->meshes = (mesh_info_t *)malloc(sizeof(mesh_info_t));
    info->meshes->mesh_id = id;
    info->mesh_count = 1;
}

internal void
create_cube(mesh_t *mesh, f32 w, f32 h, f32 d, color_t color)
{
    u32 i;
    f32 h_w = w*0.5f, h_h = h*0.5f, h_d = d*0.5f;
    
    u32 n_i[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    
    v3 *verts = mesh->verts;
    verts[0] = HMM_V3(-h_w, -h_h, -h_d);
    verts[1] = HMM_V3(h_w, -h_h, -h_d);
    verts[2] = HMM_V3(h_w, h_h, -h_d);
    verts[3] = HMM_V3(-h_w, h_h, -h_d);
    
    verts[4] = HMM_V3(h_w, -h_h, -h_d);
    verts[5] = HMM_V3(h_w, -h_h, h_d);
    verts[6] = HMM_V3(h_w, h_h, h_d);
    verts[7] = HMM_V3(h_w, h_h, -h_d);
    
    verts[8] = HMM_V3(-h_w, -h_h, h_d);
    verts[9] = HMM_V3(h_w, -h_h, h_d);
    verts[10] = HMM_V3(h_w, h_h, h_d);
    verts[11] = HMM_V3(-h_w, h_h, h_d);
    
    verts[12] = HMM_V3(-h_w, -h_h, -h_d);
    verts[13] = HMM_V3(-h_w, -h_h, h_d);
    verts[14] = HMM_V3(-h_w, h_h, h_d);
    verts[15] = HMM_V3(-h_w, h_h, -h_d);
    
    verts[16] = HMM_V3(-h_w, h_h, -h_d);
    verts[17] = HMM_V3(h_w, h_h, -h_d);
    verts[18] = HMM_V3(h_w, h_h, h_d);
    verts[19] = HMM_V3(-h_w, h_h, h_d);
    
    verts[20] = HMM_V3(-h_w, -h_h, -h_d);
    verts[21] = HMM_V3(h_w, -h_h, -h_d);
    verts[22] = HMM_V3(h_w, -h_h, h_d);
    verts[23] = HMM_V3(-h_w, -h_h, h_d);
    
    v2 *uvs = mesh->uvs;
    for(i = 0; i < 6; i+=4)
    {
        uvs[i] = HMM_V2(0.0f, 0.0f);
        uvs[i+1] = HMM_V2(1.0f, 0.0f);
        uvs[i+2] = HMM_V2(1.0f, 1.0f);
        uvs[i+3] = HMM_V2(0.0f, 1.0f);
    }
    
    color_t *colors = mesh->colors;
    for(i = 0; i < 24; i++)
        colors[i] = color;
    
    v3 n_n[] = {
        HMM_V3(0.0f, -1.0f, 0.0f),
        HMM_V3(1.0f, 0.0f, 0.0f),
        HMM_V3(0.0f, 0.0f, 1.0f),
        HMM_V3(-1.0f, 0.0f, 0.0f),
        HMM_V3(0.0f, 1.0f, 0.0f),
        HMM_V3(0.0f, 0.0f, 1.0f),
    };
    
    v3 *norms = mesh->norms;
    for(i = 0; i < 6; i++)
    {
        norms[4*i] = n_n[i];
        norms[4*i+1] = n_n[i];
        norms[4*i+2] = n_n[i];
        norms[4*i+3] = n_n[i];
    }
    
    u32 *indices = mesh->indices;
    memcpy(indices, n_i, sizeof(n_i));
}

internal void
add_cube(asset_manager_t *am, f32 w, f32 h, f32 d, color_t color, model_info_t *info)
{
    u32 id = get_stack_count(am->meshes);
    mesh_t *mesh = (mesh_t *)stack_push(&am->meshes);
    
    init_mesh(mesh, 24, 36);
    create_cube(mesh, w, h, d, color);
    
    info->meshes = (mesh_info_t *)malloc(sizeof(mesh_info_t));
    info->meshes->mesh_id = id;
    info->mesh_count = 1;
}

// TODO(ajeej): finish implementing
/*
internal void
create_sphere(gl_renderer *gl, f32 r, u32 res)
{
u32 i, j;
f32 phi, sin_phi, cos_phi;
f32 theta, sin_theta, cos_theta;
f32 x, y, z;
v3 *verts, norms;
v2 *uvs;

for(i = 0; i < res; i++)
{
    phi = HMM_PI * (f32)i/(f32)res;
    sin_phi = sin(phi);
    cos_phi = cos(phi);
    
    for(j = 0; j < res; j++)
    {
        theta = 2.0f * HMM_PI * (f32)j/(f32)res;
        sin_theta = sin(theta);
        cos_theta = cos(theta);
        
        x = cos_theta * sin_phi;
        y = cos_phi;
        z = sin_theta * sin_phi;
        
        verts = (v3 *)stack_push(&gl->verts);
        *verts = HMM_Vec3(r*x, r*y, r*z);
        
        norms = (v3 *)stack_push(&gl->norms);
        *norms = HMM_Vec3(x, y, z);
        
        uvs = (v2 *)stack_push(&gl->uvs);
        *uvs = HMM_Vec2((f32)j/(f32)res);
        *uvs = HMM_Vec2((f32)i/(f32)res);
    }
}
}
*/