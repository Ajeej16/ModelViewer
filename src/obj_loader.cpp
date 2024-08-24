\

internal void
skip_space(u8 **token)
{
    while(IS_SPACE(**token)) (*token)++;
}

internal void
skip_space_and_cr(u8 **token)
{
    while(IS_SPACE(**token) || **token == '\r') (*token)++;
}

internal void
skip_to_new_line(u8 **token)
{
    while(!IS_NEW_LINE(**token)) (*token)++;
}

internal i32
parse_int(u8 **token)
{
    i32 result = 0;
    // TODO(ajeej): check to move this out of here
    skip_space(token);
    result = strtol((char *)*token, (char **)token, 10);
    return result;
}

internal f64
parse_double(u8 **token)
{
    u8 *end;
    skip_space(token);
    f64 result = strtod((char *)*token, (char **)&end);
    *token = end;
    return result;
}

internal void
parse_v2(f32 *v, u8 **token)
{
    v[0] = parse_double(token);
    v[1] = parse_double(token);
}

internal void
parse_v3(f32 *v, u8 **token)
{
    v[0] = parse_double(token);
    v[1] = parse_double(token);
    v[2] = parse_double(token);
}

internal vert_idx_t
parse_indices(u8 **token)
{
    vert_idx_t res;
    res.v = -1; res.vn = -1; res.vt = -1;
    
    res.v = parse_int(token)-1;
    if(**token != '/')
        return res;
    (*token)++;
    
    if(**token != '/') {
        res.vt = parse_int(token)-1;
        if(**token != '/')
            return res;
    }
    (*token)++;
    res.vn = parse_int(token)-1;
    
    return res;
}

internal u32
parse_name(u8 **token, char **out_name)
{
    char *name;
    u32 length = 0; u8 *start = *token;
    while(!IS_NEW_LINE(**token))
        (*token)++;
    
    length = (*token - start);
    
    name = (char *)malloc(length+1);
    memcpy(name, start, length);
    name[length] = '\0';
    
    *out_name = name;
    
    return length;
}

internal void
parse_obj(obj_info_t *info, u8 *token,
          char **out_mtl_name)
{
    obj_t *obj = NULL;
    f32 *v, *vn, *vt;
    vert_idx_t *idx;
    
    u32 idx_offset = 0;
    *out_mtl_name = NULL;
    
    while(*token) {
        if((*token == 'o' || *token == 'g') && IS_SPACE(*(token+1))) {
            token += 2;
            
            if(obj)
                obj->idx_count = idx_offset - obj->idx_offset;
            
            obj = (obj_t *)stack_push(&info->objs);
            obj->idx_offset = idx_offset;
            parse_name(&token, &obj->name);
        } else if(*token == 'v') {
            token++;
            
            if(IS_SPACE(*token)) {
                token++;
                v = (f32 *)stack_push_array(&info->v, 3);
                parse_v3(v, &token);
            }
            else if(*token == 'n' && IS_SPACE(*(token+1))) {
                token += 2;
                vn = (f32 *)stack_push_array(&info->vn, 3);
                parse_v3(vn, &token);
            }
            else if(*token == 't' && IS_SPACE(*(token+1))) {
                token += 2;
                vt = (f32 *)stack_push_array(&info->vt, 2);
                parse_v2(vt, &token);
            }
            else
                ASSERT(0, "Unsupported Obj Command.");
        } else if (*token == 'f' && IS_SPACE(*(token+1))) {
            token += 2;
            
            u32 face_count = 0;
            vert_idx_t i[3];
            i[0] = parse_indices(&token);
            skip_space_and_cr(&token);
            i[2] = parse_indices(&token);
            skip_space_and_cr(&token);
            
            // NOTE(ajeej): generate normals
            v3 n;
            u32 is_gen_norms = 0;
            
            while(!IS_NEW_LINE(*token)) {
                i[1] = i[2];
                i[2] = parse_indices(&token);
                skip_space_and_cr(&token);
                
                idx = (vert_idx_t *)stack_push_array(&info->v_idx, 3);
                memcpy(idx, i, sizeof(*i)*3);
                face_count++;
            }
            
            idx_offset += face_count*3;
        } else if ((strncmp((char *)token, "usemtl", 6) == 0) &&
                   IS_SPACE(*(token+6))) {
            token += 7;
            
            parse_name(&token, &obj->mat_name);
        } else if ((strncmp((char *)token, "mtllib", 6) == 0) &&
                   IS_SPACE(*(token+6))) {
            token += 7;
            parse_name(&token, out_mtl_name);
        }
        
        skip_to_new_line(&token);
        token++;
    }
    
    obj->idx_count = idx_offset - obj->idx_offset;
}

internal void
init_obj_material(obj_mat_t *material)
{
    memset(material, 0, sizeof(*material));
    
    material->dissolve = 1.0f;
    material->shininess = 1.0f;
    material->index_of_refraction = 1.0f;
}

internal void
parse_mtl(obj_info_t *info, u8 *token)
{
    obj_mat_t *mat = NULL;
    
    while(*token) {
        if((strncmp((char *)token, "newmtl", 6) == 0) &&
           IS_SPACE(*(token+6))) {
            token += 7;
            
            u32 mat_id = get_stack_count(info->materials);
            mat = (obj_mat_t *)stack_push(&info->materials);
            init_obj_material(mat);
            
            parse_name(&token, &mat->name);
            
            obj_t *objs = info->objs;
            u32 obj_count = get_stack_count(objs);
            for(u32 i = 0; i < obj_count; i++) {
                if (strcmp(mat->name, objs[i].mat_name) == 0) {
                    objs[i].mat_id = mat_id;
                }
            }
        } else if (*token == 'K') {
            token++;
            
            if (*token == 'a' && IS_SPACE(*(token+1))) {
                token += 2;
                parse_v3(mat->ambient, &token);
            } else if (*token == 'd' && IS_SPACE(*(token+1))) {
                token += 2;
                parse_v3(mat->diffuse, &token);
            } else if (*token == 's' && IS_SPACE(*(token+1))) {
                token += 2;
                parse_v3(mat->specular, &token);
            } else if (*token == 't' && IS_SPACE(*(token+1))) {
                token += 2;
                parse_v3(mat->transmittance, &token);
            } else if (*token == 'e' && IS_SPACE(*(token+1))) {
                token += 2;
                parse_v3(mat->emission, &token);
            } else
                LOG("Unsupported Mtl Command.");
        } else if (*token == 'N') {
            token++;
            
            if (*token == 'i' && IS_SPACE(*(token+1))) {
                token += 2;
                mat->index_of_refraction = parse_double(&token);
            } else if (*token == 's' && IS_SPACE(*(token+1))) {
                token += 2;
                mat->shininess = parse_double(&token);
            }
            else
                LOG("Unsupported Mtl Command.");
        } else if (*token == 'd' && IS_SPACE(*(token+1))) {
            token += 2;
            mat->dissolve = parse_double(&token);
        } else if (*token == 'T' && *(token+1) == 'r' &&
                   IS_SPACE(*(token+2))) {
            token += 3;
            mat->dissolve = 1.0f - parse_double(&token);
        } else if ((strncmp((char *)token, "illum", 5) == 0) &&
                   IS_SPACE(*(token+5))) {
            token += 6;
            mat->illum = parse_int(&token);
        } else if (strncmp((char *)token, "map_", 4) == 0) {
            token += 4;
            
            if(*token == 'K') {
                token++;
                
                if(*token == 'a' && IS_SPACE(*(token+1))) {
                    token += 2;
                    parse_name(&token, &mat->ambient_texture_name);
                } else if (*token == 'd' && IS_SPACE(*(token+1))) {
                    token += 2;
                    parse_name(&token, &mat->diffuse_texture_name);
                } else if (*token == 's' && IS_SPACE(*(token+1))) {
                    token += 2;
                    parse_name(&token, &mat->specular_texture_name);
                }
                else
                    LOG("Unsupported Mtl Command.");
            } else if (*token == 'N' && *(token+1) == 's' &&
                       IS_SPACE(*(token+2))) {
                token += 3;
                parse_name(&token, &mat->specular_highlight_texture_name);
            } else if ((strncmp((char *)token, "Bump", 4) == 0) && 
                       IS_SPACE(*(token+4))) {
                token += 5;
                parse_name(&token, &mat->bump_texture_name);
            } else if (*token == 'd' && IS_SPACE(*(token+1))) {
                token += 2;
                parse_name(&token, &mat->alpha_texture_name);
            }
            else
                LOG("Unsupported Mtl Command.");
        } else if ((strncmp((char *)token, "bump", 4) == 0) &&
                   IS_SPACE(*(token+4))) {
            token += 5;
            parse_name(&token, &mat->bump_texture_name);
        } else if ((strncmp((char *)token, "disp", 4) == 0) &&
                   IS_SPACE(*(token+4))) {
            token += 5;
            parse_name(&token, &mat->displacement_texture_name);
        }
        
        skip_to_new_line(&token);
        token++;
    }
}

internal obj_info_t
load_obj(char *filename)
{
    char path[PATH_MAX] = "..\\data\\models\\";
    
    ASSERT(strlen(path)+strlen(filename) < sizeof(path),
           "Shader file path is too long.");
    strcat(path, filename);
    
    u8 *obj_data;
    read_file(path, &obj_data);
    
    char *mtl_name = NULL;
    obj_info_t info = {0};
    
    char *last_slash = strrchr(path, '\\');
    info.path = (char *)malloc(last_slash-path+2);
    memcpy(info.path, path, last_slash-path+1);
    info.path[last_slash-path+1] = '\0';
    
    if(obj_data)
        parse_obj(&info, obj_data, &mtl_name);
    
    u8 *mtl_data = NULL;
    if(mtl_name) {
        strcpy(last_slash+1, mtl_name);
        read_file(path, &mtl_data);
    }
    
    if(mtl_data)
        parse_mtl(&info, mtl_data);
    
    return info;
}

internal void
free_obj(obj_info_t *info)
{
    free(info->path);
    
    stack_free(info->v);
    if(info->vn)
        stack_free(info->vn);
    if(info->vt)
        stack_free(info->vt);
    stack_free(info->idx);
    
    u32 i;
    u32 obj_count = get_stack_count(info->objs);
    u32 mat_count = get_stack_count(info->materials);
    obj_t *obj = info->objs;
    obj_mat_t *mat = info->materials;
    
    for(i = 0; i < obj_count; i++, obj++)
        free(obj->name);
    for(i = 0; i < mat_count; i++, mat++) {
        free(mat->name);
        
        if(mat->ambient_texture_name)
            free(mat->ambient_texture_name);
        if(mat->diffuse_texture_name)
            free(mat->diffuse_texture_name);
        if(mat->specular_texture_name)
            free(mat->specular_texture_name);
        if(mat->specular_highlight_texture_name)
            free(mat->specular_highlight_texture_name);
        if(mat->bump_texture_name)
            free(mat->bump_texture_name);
        if(mat->displacement_texture_name)
            free(mat->displacement_texture_name);
        if(mat->alpha_texture_name)
            free(mat->alpha_texture_name);
    }
    
    if(info->objs)
        stack_free(info->objs);
    if(info->materials)
        stack_free(info->materials);
}