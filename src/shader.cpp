
internal char *
load_shader_source(const char *filename)
{
    char path[PATH_MAX] = "..\\data\\shaders\\";
    
    ASSERT(strlen(path)+strlen(filename) < sizeof(path),
           "Shader file path is too long.");
    strcat(path, filename);
    
    FILE *file = fopen(path, "r");
    ASSERT(file != NULL, "Could not open shader source file.");
    
    u64 size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *data = (char *)malloc(size+1);
    fread(data, 1, size, file);
    data[size] = '\0';
    
    fclose(file);
    
    return data;
}

internal u32
compile_shader(char *src, u32 type)
{
    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        
        LOG("ERROR::SHADER::COMPILATION: %s", info_log);
        
        return 0;
    }
    
    return shader;
}

internal u32
compile_shader_from_file(const char *filename, u32 type)
{
    char *src = load_shader_source(filename);
    u32 shader = compile_shader(src, type);
    
    free(src);
    return shader;
}

internal u32
create_program(u32 *shaders, u32 count)
{
    u32 program = glCreateProgram();
    
    for(u32 i = 0; i < count; i++)
        glAttachShader(program, shaders[i]);
    glLinkProgram(program);
    
    u32 success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, (GLint *)&success);
    
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        
        LOG("ERROR::SHADER::LINKING_FAILED: %s", info_log);
        
        return 0;
    }
    
    return program;
}

internal u32
add_shader(gl_renderer *gl, u32 count, ...)
{
    u32 id = get_stack_count(gl->shaders);
    u32 *shader = (u32 *)stack_push(&gl->shaders);
    
    u32 *s = (u32 *)malloc(count*sizeof(*s));
    
    va_list args;
    va_start(args, count);
    
    char *src;
    u32 type;
    for(u32 i = 0; i < count; i++) {
        src = va_arg(args, char *);
        type = va_arg(args, u32);
        s[i] = compile_shader_from_file(src, type);
    }
    
    *shader = create_program(s, count);
    free(s);
    
    return id;
}
