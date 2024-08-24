
internal void
init_gl_renderer(gl_renderer *gl, camera_t *camera)
{
    glGenVertexArrays(1, &gl->vao);
    glGenBuffers(VBO_COUNT, gl->vbos);
    
    gl->verts = NULL;
    gl->uvs = NULL;
    gl->colors = NULL;
    gl->indices = NULL;
    
    gl->cmds = NULL;
    gl->transforms = NULL;
    gl->entities = NULL;
    
    gl->shaders = NULL;
    
    gl->camera = camera;
    gl->light_count = 0;
    
    gl->gpu_transform = 1;
    gl->use_textures = 1;
}

internal void
free_gl_renderer(gl_renderer *gl)
{
    stack_free(gl->verts);
    stack_free(gl->uvs);
    stack_free(gl->colors);
    stack_free(gl->indices);
    stack_free(gl->cmds);
    stack_free(gl->transforms);
    stack_free(gl->shaders);
    
    glDeleteVertexArrays(1, &gl->vao);
    glDeleteBuffers(VBO_COUNT, gl->vbos);
}

internal void
update_camera(camera_t *camera)
{
    camera->proj = HMM_Perspective_RH_NO(HMM_ToRad(camera->fov), camera->aspect_ratio, camera->near, camera->far);
    
    f32 yaw = camera->rot.X, pitch = camera->rot.Y;
    
    v3 front;
    front.X = cos(HMM_ToRad(yaw))*cos(HMM_ToRad(pitch));
    front.Y = sin(HMM_ToRad(pitch));
    front.Z = sin(HMM_ToRad(yaw))*cos(HMM_ToRad(pitch));
    
    camera->front = HMM_NormV3(front);
    camera->right = HMM_NormV3(HMM_Cross(camera->front, camera->world_up));
    camera->up = HMM_NormV3(HMM_Cross(camera->right, camera->front));
}

internal void
init_camera(camera_t *cam, v3 pos, f32 speed, f32 sens, 
            f32 aspect_ratio, f32 fov = 45.0f, f32 near = 0.1f, f32 far = 50.0f)
{
    cam->pos = pos;
    cam->world_up = HMM_V3(0.0f, 1.0f, 0.0f);
    cam->rot = HMM_V3(-90.0f, 0.0f, 0.0f);
    cam->speed = speed;
    cam->sens = sens;
    cam->fov = fov;
    cam->aspect_ratio = aspect_ratio;
    cam->near = near;
    cam->far = far;
    
    update_camera(cam);
}

internal void
init_light(light_t *light, v3 pos, f32 constant = 1.0f, 
           f32 linear = 0.09f, f32 quadratic = 0.032f) 
{
    light->pos = pos;
    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;
}

internal void
add_light(gl_renderer *gl, v3 pos, f32 constant = 1.0f, 
          f32 linear = 0.09f, f32 quadratic = 0.032f) 
{
    init_light(gl->lights+(gl->light_count++),
               pos, constant, linear, quadratic);
}

void
init_framebuffer(gl_renderer *gl, const char *vert, const char *frag)
{
    u32 fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    
    u32 fb_tex;
    glGenTextures(1, &fb_tex);
    glBindTexture(GL_TEXTURE_2D, fb_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SRC_WIDTH, SRC_HEIGHT, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0);
    
    u32 rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SRC_WIDTH, SRC_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed to make framebuffer.");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    gl->fb.id = fb;
    gl->fb.rbo = rbo;
    gl->fb.tex = fb_tex;
    
    u32 shader = add_shader(gl, 2, vert, GL_VERTEX_SHADER, frag, GL_FRAGMENT_SHADER);
    gl->fb.shader = shader;
    
    f32 quad[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,-1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f
    };
    u32 vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
                          (void *)(2*sizeof(float)));
    
    gl->fb.vao = vao;
}

/*internal render_cmd_t *
push_render_command(gl_renderer *gl, u32 indices_count, 
                    u32 trans_id, u32 mat_id = 0, u32 prim_type = PRIMITIVE_TRIANGLES)
{
    render_cmd_t *cmd = (render_cmd_t *)stack_push(&gl->cmds);
    cmd->indices_idx = get_stack_count(gl->indices);
    cmd->indices_count = indices_count;
    cmd->mat_id = mat_id;
    cmd->trans_id = trans_id;
    cmd->prim_type = prim_type;
    
    return cmd;
}*/

internal void
update_transform(gl_renderer *gl, mat4 *transform, v3 pos, v3 rot, v3 scale)
{
    mat4 trans = HMM_M4D(1.0f);
    trans = HMM_MulM4(trans, HMM_Translate(pos));
    
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(rot.X), HMM_V3(1.0f, 0.0f, 0.0f)));
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(rot.Y), HMM_V3(0.0f, 1.0f, 0.0f)));
    trans = HMM_MulM4(trans, HMM_Rotate_RH(HMM_ToRad(rot.Z), HMM_V3(0.0f, 0.0f, 1.0f)));
    
    trans = HMM_MulM4(trans, HMM_Scale(scale));
    
    *transform = trans;
}

internal u32
add_entity(gl_renderer *gl, model_info_t *m_info, 
           v3 pos = HMM_V3(0.0f, 0.0f, 0.0f), v3 rot = HMM_V3(0.0f, 0.0f, 0.0f),
           v3 scale = HMM_V3(1.0f, 1.0f, 1.0f))
{
    u32 id = get_stack_count(gl->entities);
    
    entity_t *entity = (entity_t *)stack_push(&gl->entities);
    entity->pos = pos;
    entity->rot = rot;
    entity->scale = scale;
    
    m_info->trans_id = get_stack_count(gl->transforms);
    
    mat4 *transform = (mat4 *)stack_push(&gl->transforms);
    update_transform(gl, transform, pos, rot, scale);
    
    entity->m_info = *m_info;
    
    return id;
}


internal void
push_mesh(gl_renderer *gl, asset_manager_t *am,
          model_info_t *m_info, color_t color = color_t{ 0, 0, 0, 255})
{
    mesh_t mesh;
    mesh_info_t *mesh_info;
    u32 model_idx_offset = get_stack_count(gl->verts);
    for(u32 m_idx = 0; m_idx < m_info->mesh_count; m_idx++) {
        mesh_info = m_info->meshes+m_idx;
        mesh = am->meshes[mesh_info->mesh_id];
        u32 v_count = mesh.vert_count, i_count = mesh.indices_count;
        
        u32 idx_offset = get_stack_count(gl->indices);
        mesh_info->vert_idx = get_stack_count(gl->verts);
        mesh_info->indices_idx = idx_offset;
        mesh_info->indices_count = i_count;
        
        v3 *verts = (v3 *)stack_push_array(&gl->verts, v_count);
        v2 *uvs = (v2 *)stack_push_array(&gl->uvs, v_count);
        color_t *colors = (color_t *)stack_push_array(&gl->colors, v_count);
        v3 *norms = (v3 *)stack_push_array(&gl->norms, v_count);
        u32 *indices = (u32 *)stack_push_array(&gl->indices, i_count);
        
        memcpy(verts, mesh.verts, v_count*sizeof(v3));
        memcpy(uvs, mesh.uvs, v_count*sizeof(v2));
        memcpy(norms, mesh.norms, v_count*sizeof(v3));
        
        for(u32 i = 0; i < v_count; i++)
            colors[i] = color;
        for(u32 i = 0; i < i_count; i++)
            indices[i] = mesh.indices[i]+model_idx_offset;
    }
}

internal void
push_curve(gl_renderer *gl, v3 *p, u32 p_count, color_t color = color_t{ 0, 0, 0, 255})
{
    render_cmd_t *cmd = (render_cmd_t *)stack_push(&gl->cmds);
    
    u32 indices_idx = get_stack_count(gl->indices);
    cmd->mesh_id = 0;
    cmd->trans_id = 0;
    cmd->vert_idx = get_stack_count(gl->verts);
    cmd->indices_idx = indices_idx;
    cmd->indices_count = p_count;
    cmd->prim_type = PRIMITIVE_LINE_STRIP;
    
    v3 *verts = (v3 *)stack_push_array(&gl->verts, p_count);
    v2 *uvs = (v2 *)stack_push_array(&gl->uvs, p_count);
    color_t *colors = (color_t *)stack_push_array(&gl->colors, p_count);
    v3 *norms = (v3 *)stack_push_array(&gl->norms, p_count);
    u32 *indices = (u32 *)stack_push_array(&gl->indices, p_count);
    
    memcpy(verts, p, p_count*sizeof(v3));
    memset(uvs, 0, p_count*sizeof(v2));
    memset(norms, 0, p_count*sizeof(v3));
    
    for (u32 i = 0; i < p_count; i++)
        colors[i] = color;
    
    for (u32 i = 0; i < p_count; i++)
        indices[i] = i + indices_idx;
}


internal void
matrix_bezier_cubic(v3 *p, f32 dt, v3 **rp)
{
    mat4 m = mat4{
        -1, 3, -3, 1,
        3, -6, 3, 0,
        -3, 3, 0, 0,
        1, 0, 0, 0,
    };
    
    mat4 pm = {0};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 3; j++)
        {
            pm[i][j] = p[i][j];
        }
        
        pm[i][3] = 1.0f;
    }
    
    mat4 bm = HMM_MulM4(pm, m);
    
    v4 tv;
    v3 *r;
    for (f32 t = 0.0f; t < 1.0f; t += dt)
    {
        tv = HMM_V4(t*t*t, t*t, t, 1);
        r = (v3 *)stack_push(rp);
        *r = HMM_MulM4V4(bm, tv).XYZ;
    }
    
    r = (v3 *)stack_push(rp);
    *r = p[3];
}


internal void
de_castelgau(v3 *p, u32 n, f32 dt, v3 **rp, u32 debug, f32 debug_t,
             gl_renderer *gl)
{
    v3 *res, *q = (v3 *)malloc(n*sizeof(v3));
    v3 l, r;
    
    u32 debug_draw = 0;
    
    for (f32 t = 0.0f; t < 1.0f; t += dt) {
        memcpy(q, p, n*sizeof(v3));
        
        if (debug) {
            if (debug_t <= t)
                debug_draw = 1;
            else if (MIN(debug_t+dt, 1.0f) == 1.0f && MIN(t+dt, 1.0f) == 1.0f) 
                push_curve(gl, q, 4, color_t{ 125, 0, 0, 255 });
        }
        
        
        for (u32 k = 1; k < n; k++) {
            if (debug_draw)
                push_curve(gl, q, n-k+1, color_t{ 125, 0, 0, 255 });
            
            for (u32 i = 0; i < n-k; i++) {
                l = HMM_MulV3F(q[i], (1-t));
                r = HMM_MulV3F(q[i+1], t);
                
                q[i] = HMM_AddV3(l, r);
            }
        }
        
        res = (v3 *)stack_push(rp);
        *res = q[0];
        
        if (debug_draw)
            debug = 0;
        
        debug_draw = 0;
    }
    
    free(q);
    
    res = (v3 *)stack_push(rp);
    *res = p[n-1];
}

internal void
adjust_bezier_c1(v3 *p, u32 p_id)
{
    u32 or = p_id % 3;
    
    v3 moved_p, m, *change_p;
    
    switch (or) {
        case 0: {
            moved_p = p[p_id-1];
            m = p[p_id];
            change_p = p+p_id+1;
        } break;
        case 1: {
            moved_p = p[p_id];
            m = p[p_id-1];
            change_p = p+p_id-2;
        } break;
        case 2: {
            moved_p = p[p_id];
            m = p[p_id+1];
            change_p = p+p_id+2;
        } break;
    }
    
    *change_p = HMM_AddV3(m, HMM_SubV3(m, moved_p));
}


internal void
adjust_bezier_g1(v3 *p, u32 p_id)
{
    u32 or = p_id % 3;
    
    v3 moved_p, m, *change_p;
    
    switch (or) {
        case 0: {
            moved_p= p[p_id-1];
            m= p[p_id];
            change_p= p+p_id+1;
        } break;
        case 1: {
            moved_p = p[p_id];
            m = p[p_id-1];
            change_p = p+p_id-2;
        } break;
        case 2: {
            moved_p = p[p_id];
            m = p[p_id+1];
            change_p = p+p_id+2;
        } break;
    }
    
    v3 d = HMM_NormV3(HMM_SubV3(moved_p, m));
    
    f32 mag = HMM_DotV3(d, HMM_SubV3(*change_p, m));
    if (mag > 0.0f)
        mag = -mag;
    
    *change_p = HMM_AddV3(m, HMM_MulV3F(d, mag));
}

internal void
render_mesh(gl_renderer *gl, model_info_t m_info)
{
    mesh_info_t mesh_info;
    
    render_cmd_t *cmd = (render_cmd_t *)stack_push_array(&gl->cmds,
                                                         m_info.mesh_count);
    for(u32 m_idx = 0; m_idx < m_info.mesh_count; m_idx++, cmd++) {
        mesh_info = m_info.meshes[m_idx];
        
        cmd->mesh_id = mesh_info.mesh_id;
        cmd->trans_id = m_info.trans_id;
        cmd->vert_idx = mesh_info.vert_idx;
        cmd->indices_idx = mesh_info.indices_idx;
        cmd->indices_count = mesh_info.indices_count;
        cmd->prim_type = PRIMITIVE_TRIANGLES;
    }
}

internal void
render_entities(gl_renderer *gl)
{
    u32 count = get_stack_count(gl->entities);
    entity_t *entities = gl->entities;
    for(int i = 0; i < count; i++, entities++)
        render_mesh(gl, entities->m_info);
}

internal void
bind_material(asset_manager_t *am, material_t *mat, u32 p_id, u32 use_texture)
{
    set_uniform_vec3(p_id, "mat.ambient", *(v3 *)mat->ambient);
    set_uniform_vec3(p_id, "mat.diffuse", *(v3 *)mat->diffuse);
    set_uniform_vec3(p_id, "mat.specular", *(v3 *)mat->specular);
    set_uniform_float(p_id, "mat.shininess", mat->shininess);
    
    u32 idx = (use_texture) ? mat->diffuse_tex_id : 0;
    texture_t tex = am->textures[idx];
    glActiveTexture(GL_TEXTURE0);
    
    set_uniform_int(p_id, "mat.diffuse_tex", 0);
    
    glBindTexture(GL_TEXTURE_2D, tex.id);
    
    idx = (use_texture) ? mat->specular_tex_id : 0;
    tex = am->textures[idx];
    glActiveTexture(GL_TEXTURE1);
    
    set_uniform_int(p_id, "mat.specular_tex", 1);
    
    glBindTexture(GL_TEXTURE_2D, tex.id);
}

internal void
bind_lights(light_t *lights, u32 light_count, u32 p_id)
{
    set_uniform_int(p_id, "point_light_count", light_count);
    
    char buffer[32];
    for(u32 i = 0; i < light_count; i++) {
        sprintf(buffer, "point_lights[%u].pos", i);
        set_uniform_vec3(p_id, buffer, lights[i].pos);
        
        sprintf(buffer, "point_lights[%u].constant", i);
        set_uniform_float(p_id, buffer, lights[i].constant);
        sprintf(buffer, "point_lights[%u].linear", i);
        set_uniform_float(p_id, buffer, lights[i].linear);
        sprintf(buffer, "point_lights[%u].quadratic", i);
        set_uniform_float(p_id, buffer, lights[i].quadratic);
    }
}

internal void
render(gl_renderer *gl, asset_manager_t *am, u32 p_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, gl->fb.id);
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    u32 cmd_count = get_stack_count(gl->cmds);
    render_cmd_t *cmd;
    
    camera_t *cam = gl->camera;
    mat4 view = HMM_LookAt_RH(cam->pos, HMM_AddV3(cam->pos, cam->front), cam->up);
    
    
    glBindVertexArray(gl->vao);
    
    if(gl->gpu_transform) {
        glBindBuffer(GL_ARRAY_BUFFER, gl->vbos[VBO_VERTS]);
        glBufferData(GL_ARRAY_BUFFER, get_stack_size(gl->verts),
                     (f32 *)gl->verts, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(0);
    }
    else 
    {
        cmd = gl->cmds;
        
        mat4 vp = HMM_MulM4(cam->proj, view);
        mat4 mvp;
        
        v3 *verts = (v3 *)malloc(get_stack_size(gl->verts));
        memcpy(verts, gl->verts, get_stack_size(gl->verts));
        
        for(u32 i = 0; i < cmd_count; i++, cmd++) {
            u32 v_count = am->meshes[cmd->mesh_id].vert_count;
            u32 idx = cmd->vert_idx;
            
            mvp = HMM_MulM4(vp, gl->transforms[cmd->trans_id]);
            
            for(u32 i = 0; i < v_count; i++) {
                v4 temp =HMM_MulM4V4(mvp, HMM_V4V(gl->verts[i+idx], 1.0f));
                verts[i+idx] = HMM_DivV3F(temp.XYZ, temp.W);
            }
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, gl->vbos[VBO_VERTS]);
        glBufferData(GL_ARRAY_BUFFER, get_stack_size(gl->verts),
                     (f32 *)verts, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(0);
        
        free(verts);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, gl->vbos[VBO_UVS]);
    glBufferData(GL_ARRAY_BUFFER, get_stack_size(gl->uvs),
                 (f32 *)gl->uvs, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, gl->vbos[VBO_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, get_stack_size(gl->colors),
                 (f32 *)gl->colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, gl->vbos[VBO_NORMS]);
    glBufferData(GL_ARRAY_BUFFER, get_stack_size(gl->norms),
                 (f32 *)gl->norms, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(3);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl->vbos[VBO_INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, get_stack_size(gl->indices),
                 gl->indices, GL_DYNAMIC_DRAW);
    
    u32 shader = get_shader(gl, p_id);
    glUseProgram(shader);
    
    if(gl->gpu_transform) {
        set_uniform_mat4(shader, "proj", cam->proj);
        set_uniform_mat4(shader, "view", view);
    }
    else {
        set_uniform_mat4(shader, "proj", HMM_M4D(1.0f));
        set_uniform_mat4(shader, "view", HMM_M4D(1.0f));
    }
    
    bind_lights(gl->lights, gl->light_count, shader);
    
    set_uniform_float(shader, "near", cam->near);
    set_uniform_float(shader, "far", cam->far);
    
    cmd = gl->cmds;
    for(u32 i = 0; i < cmd_count; i++, cmd++) {
        
        material_t *mat = get_mesh_material(am, cmd->mesh_id);
        if(mat)
            bind_material(am, mat, shader, gl->use_textures);
        
        if(gl->gpu_transform) {
            set_uniform_mat4(shader, "model", gl->transforms[cmd->trans_id]);
        }
        else {
            set_uniform_mat4(shader, "model", HMM_M4D(1.0f));
        }
        
        glDrawElements(cmd->prim_type, cmd->indices_count,
                       GL_UNSIGNED_INT, (void *)(cmd->indices_idx*sizeof(u32)));
    }
    
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

internal void
render_framebuffer(gl_renderer *gl)
{
    framebuffer_t fb = gl->fb;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    u32 shader = get_shader(gl, fb.shader);
    glUseProgram(shader);
    
    glBindVertexArray(fb.vao);
    
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb.tex);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}