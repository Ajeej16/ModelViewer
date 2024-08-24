
internal void
set_uniform_int(u32 id, const char *name, i32 val) {
    i32 loc = glGetUniformLocation(id, name);
    if (loc == -1)
        return;
    
    glUniform1i(loc, val);
}

internal void
set_uniform_float(u32 id, const char *name, f32 val) {
    i32 loc = glGetUniformLocation(id, name);
    if (loc == -1)
        return;
    
    glUniform1f(loc, val);
}

internal void
set_uniform_vec2(u32 id, const char *name, v2 val) {
    i32 loc = glGetUniformLocation(id, name);
    if (loc == -1)
        return;
    
    glUniform2f(loc, val.X, val.Y);
}

internal void
set_uniform_vec3(u32 id, const char *name, v3 val) {
    i32 loc = glGetUniformLocation(id, name);
    if (loc == -1)
        return;
    
    glUniform3f(loc, val.X, val.Y, val.Z);
}

internal void
set_uniform_vec4(u32 id, const char *name, v4 val) {
    i32 loc = glGetUniformLocation(id, name);
    if (loc == -1)
        return;
    
    glUniform4f(loc, val.X, val.Y, val.Z, val.W);
}

internal void
set_uniform_mat4(u32 id, const char *name, mat4 val) {
    i32 loc = glGetUniformLocation(id, name);
    if (loc == -1)
        return;
    
    glUniformMatrix4fv(loc , 1, GL_FALSE, (f32 *)val.Elements);
}