#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define HMM_ANGLE_USER_TO_INTERNAL
#define HANDMADE_MATH_USE_DEGREES
#include <HandmadeMath.h>

#include "utils.h"

const u32 SRC_WIDTH = 800;
const u32 SRC_HEIGHT = 600;

#include "obj_loader.h"
#include "renderer.h"
#include "assets.h"

#include "obj_loader.cpp"
#include "shader.cpp"
#include "uniform.cpp"
#include "assets.cpp"
#include "renderer.cpp"


typedef struct shader_combo_item_t {
    char *name;
    u32 shader;
} shader_combo_item_t;

#define SHADER_COMBO(n, s) shader_combo_item_t{ n, s }


void framebuffer_size_callback(GLFWwindow *window, i32 width, i32 height)
{
    glViewport(0, 0, width, height);
}

internal void
draw_vec3_widget(const char *name, v3 *value) {
    ImGui::PushID(name);
    
    f32 item_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 4;
    
    ImGui::Text(name);
    
    ImGui::PushItemWidth(item_width);
    ImGui::DragFloat("X", &value->X, 0.1f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    
    ImGui::PushItemWidth(item_width);
    ImGui::DragFloat("Y", &value->Y, 0.1f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    
    ImGui::PushItemWidth(item_width);
    ImGui::DragFloat("Z", &value->Z, 0.1f);
    ImGui::PopItemWidth();
    
    ImGui::PopID();
}

void process_input(GLFWwindow *window, gl_renderer *gl, u32 select)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    u32 update = 0;
    f32 mov_speed = 0.01f;
    f32 rot_speed = 0.5f;
    
    
    entity_t *entity = gl->entities+select;
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        entity->rot.X += rot_speed;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        entity->rot.X -= rot_speed;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
            entity->rot.Z += rot_speed;
        else
            entity->rot.Y += rot_speed;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
            entity->rot.Z -= rot_speed;
        else
            entity->rot.Y -= rot_speed;
    }
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        entity->pos.Z += mov_speed;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        entity->pos.Z -= mov_speed;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        entity->pos.X += mov_speed;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        entity->pos.X -= mov_speed;
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        entity->pos.Y += mov_speed;
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        entity->pos.Y -= mov_speed;
    
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        entity->scale.Z += mov_speed;
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        entity->scale.Z -= mov_speed;
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        entity->scale.X += mov_speed;
    if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        entity->scale.X -= mov_speed;
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        entity->scale.Y += mov_speed;
    if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        entity->scale.Y -= mov_speed;
    
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        gl->gpu_transform = 1;
    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        gl->gpu_transform = 0;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow *window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, 
                                          "Model Transforms",
                                          NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glewInit();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    
    camera_t camera = {0};
    init_camera(&camera, HMM_V3(0.0f, 0.0f, 10.0f), 1.0f, 1.0f,
                (f32)SRC_WIDTH/SRC_HEIGHT);
    
    gl_renderer gl = {0};
    init_gl_renderer(&gl, &camera);
    init_framebuffer(&gl, "framebuffer_vert.glsl", "framebuffer_frag.glsl");
    
    add_light(&gl, HMM_V3(5.0f, 2.0f, 3.0f));
    
    asset_manager_t assets = {0};
    init_asset_manager(&assets);
    
    u32 no_shade_id = add_shader(&gl, 2, "vert.glsl", GL_VERTEX_SHADER,
                                 "frag.glsl", GL_FRAGMENT_SHADER);
    
    u32 flat_id = add_shader(&gl, 3, "flat_vert.glsl", GL_VERTEX_SHADER, "flat_geo.glsl", GL_GEOMETRY_SHADER, "flat_frag.glsl", GL_FRAGMENT_SHADER);
    
    u32 gouraud_id = add_shader(&gl, 2, "gouraud_vert.glsl", GL_VERTEX_SHADER,
                                "gouraud_frag.glsl", GL_FRAGMENT_SHADER);
    
    u32 phong_id = add_shader(&gl, 2, "phong_vert.glsl", GL_VERTEX_SHADER,
                              "phong_frag.glsl", GL_FRAGMENT_SHADER);
    
    u32 z_id = add_shader(&gl, 2, "depth_vert.glsl", GL_VERTEX_SHADER,
                          "depth_frag.glsl", GL_FRAGMENT_SHADER);
    
    u32 z_prime_id = add_shader(&gl, 2, "linear_depth_vert.glsl", GL_VERTEX_SHADER,
                                "linear_depth_frag.glsl", GL_FRAGMENT_SHADER);
    
    
    u32 shader = get_shader(&gl, gl.fb.id);
    glUseProgram(shader);
    set_uniform_int(shader, "tex", 0);
    
    
    char *paths[4] = {
        "backpack\\backpack.obj",
        "smooth_monkey\\monkey.obj",
        "dragon\\dragon.obj",
        "cow\\cow.obj"
    };
    
    color_t colors[4] {
        color_t{255, 255, 255, 255},
        color_t{ 135, 0, 0, 255 },
        color_t{255, 255, 255, 255},
        color_t{ 0, 100, 135, 255 },
    };
    
    char *combo_models[4] = {
        "Backpack",
        "Monkey",
        "Dragon",
        "Cow"
    };
    
    model_info_t models[ARRAY_COUNT(combo_models)];
    
    for(u32 i = 0; i < ARRAY_COUNT(combo_models); i++) {
        add_model(&assets, paths[i], models+i);
        add_entity(&gl, models+i, HMM_V3(0.0f, 0.0f, 0.0f),
                   HMM_V3(0.0f, 0.0f, 0.0f),  HMM_V3(1.0f, 1.0f, 1.0f));
    }
    
    shader_combo_item_t shader_combo_items[] = {
        SHADER_COMBO("None", no_shade_id),
        SHADER_COMBO("Gouraud", gouraud_id),
        SHADER_COMBO("Phong", phong_id),
        SHADER_COMBO("Flat", flat_id),
        SHADER_COMBO("Z Buffer", z_id),
        SHADER_COMBO("Z' Buffer", z_prime_id),
    };
    
    u32 shader_combo_idx = 0, model_combo_idx = 0;
    
    v4 color[ARRAY_COUNT(combo_models)];
    for (u32 i = 0; i < ARRAY_COUNT(combo_models); i++) {
        color[i].X = colors[i].r/255;
        color[i].Y = colors[i].g/255;
        color[i].Z = colors[i].b/255;
        color[i].W = colors[i].a/255;
    }
    
    f64 last_time = glfwGetTime();
    u32 frame_count = 0;
    
    v3 *p = NULL;
    p = (v3 *)stack_push_array(&p, 4);
    p[0] = HMM_V3(0.0f, 0.0f, 0.0f);
    p[1] = HMM_V3(0.0f, 5.0f, 0.0f);
    p[2] = HMM_V3(5.0f, 5.0f, 0.0f);
    p[3] = HMM_V3(7.0f, 2.5f, 0.0f);
    
    char *c_type[] = { "None", "C1", "G1" };
    u32 c_selected = 0;
    char *a_type[] = { "De Castelgau", "Matrix Form" };
    u32 a_selected = 0;
    f32 t = 0.05f;
    f32 debug_t = 0.5f;
    u32 debug = 0;
    
    glLineWidth(3.0f);
    
    while(!glfwWindowShouldClose(window)) {
        
        process_input(window, &gl, model_combo_idx);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
        
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("InvisibleWindow", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        ImGuiID dock_space_id = ImGui::GetID("InvisibleWindowSockSpace");
        ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();
        
        
        push_mesh(&gl, &assets, models+model_combo_idx, colors[model_combo_idx]);
        
        render_mesh(&gl, gl.entities[model_combo_idx].m_info);
        
        v3 *rp = NULL;
        
        /*if (a_selected == 0) {
            u32 p_count = get_stack_count(p);
            u32 cp_count;
            for (u32 i = 0; i < p_count-1; i += 3) {
                cp_count = MIN(p_count-i, 4);
                de_castelgau(p+i, cp_count, t, &rp, 0, 0, NULL);
                
                push_curve(&gl, rp, get_stack_count(rp), color_t{255, 255, 255, 255});
                stack_clear(rp);
                
                de_castelgau(p+i, cp_count, t, &rp, debug, debug_t, &gl);
            }
        }
        else {
            u32 p_count = get_stack_count(p);
            for (u32 i = 4; i <= p_count; i += 3) {
                matrix_bezier_cubic(p+i-4, t, &rp);
                
                push_curve(&gl, rp, get_stack_count(rp), color_t{255, 255, 255, 255});
                if (debug)
                    push_curve(&gl, p, 4, color_t{125, 0, 0, 255});
            }
        }*/
        
        stack_clear(rp);
        
        render(&gl, &assets, shader_combo_items[shader_combo_idx].shader);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        ImGui::Begin("Scene");
        ImVec2 size = ImGui::GetContentRegionAvail();
        
        camera.aspect_ratio = size.x/size.y;
        
        ImGui::Image((ImTextureID)gl.fb.tex, ImVec2(size.x, size.y), 
                     ImVec2(0, 1), ImVec2(1, 0));
        
        ImGui::End();
        
        ImGui::Begin("Properties");
        
        if (ImGui::CollapsingHeader("Perspective"))
        {
            ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 90.0f);
            ImGui::DragFloat("Near Plane", &camera.near, 0.1f, 0.1f, 100.0f);
            ImGui::DragFloat("Far Plane", &camera.far, 0.1f, 0.1f, 100.0f);
        }
        
        if (ImGui::CollapsingHeader("Shader")) {
            if (ImGui::Button("Add Light")) {
                if (gl.light_count < 6)
                    add_light(&gl, HMM_V3(0.0f, 0.0f, 0.0f));
            }
            
            u32 light_count = gl.light_count;
            char buffer[32];
            for (u32 i = 0; i < light_count; i++) {
                sprintf(buffer, "Light Pos %u", i);
                draw_vec3_widget(buffer, &gl.lights[i].pos);
            }
            
            if (ImGui::BeginCombo("Shaders", shader_combo_items[shader_combo_idx].name))
            {
                for(u32 n = 0; n < ARRAY_COUNT(shader_combo_items); n++)
                {
                    bool is_selected = shader_combo_idx == n;
                    if (ImGui::Selectable(shader_combo_items[n].name, is_selected))
                        shader_combo_idx = n;
                    
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                
                ImGui::EndCombo();
            }
        }
        
        entity_t *entity = gl.entities+model_combo_idx;
        if (ImGui::CollapsingHeader("Model")) {
            if (ImGui::BeginCombo("Models", combo_models[model_combo_idx]))
            {
                for(u32 n = 0; n < ARRAY_COUNT(combo_models); n++)
                {
                    bool is_selected = model_combo_idx == n;
                    if (ImGui::Selectable(combo_models[n], is_selected))
                        model_combo_idx = n;
                    
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                
                ImGui::EndCombo();
            }
            
            draw_vec3_widget("Pos", &entity->pos);
            draw_vec3_widget("Rot", &entity->rot);
            draw_vec3_widget("Scale", &entity->scale);
            
            ImGui::ColorPicker4("Color", (float *)color[model_combo_idx].Elements, ImGuiColorEditFlags_Float);
            color_t temp; v4 tempf = color[model_combo_idx];
            temp.r = tempf.X*255; temp.g = tempf.Y*255; temp.b = tempf.Z*255; temp.a = tempf.W*255;
            colors[model_combo_idx] = temp;
            
            ImGui::Checkbox("Use Textures", (bool *)&gl.use_textures);
            
        }
        
        if (ImGui::CollapsingHeader("Curves", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("dt", &t, 0.001f, 1.0f);
            
            ImGui::Checkbox("Debug", (bool *)&debug);
            ImGui::SliderFloat("Debug t", &debug_t, 0.0f, 1.0f);
            
            for (u32 i = 0; i < ARRAY_COUNT(c_type); i++) 
            {
                if (ImGui::RadioButton(c_type[i], c_selected == i))
                    c_selected= i;
            }
            
            ImGui::Spacing();
            
            for (u32 i = 0; i < ARRAY_COUNT(a_type); i++) 
            {
                if (ImGui::RadioButton(a_type[i], a_selected == i))
                    a_selected= i;
            }
            
            char buffer[32];
            v3 prev_p;
            u32 p_count = get_stack_count(p);
            u32 c_count = ceilf((p_count-1)/3.0f);
            for (u32 i = 0; i < p_count; i++) {
                sprintf(buffer, "Pos %d", i);
                
                prev_p = p[i];
                draw_vec3_widget(buffer, p+i);
                
                if (c_selected != 0 && i > 1 && i < c_count*3-1 && 
                    (prev_p.X != p[i].X || prev_p.Y != p[i].Y || prev_p.Z != p[i].Z)) 
                {
                    if (c_selected == 1) {
                        adjust_bezier_c1(p, i);
                    }
                    else {
                        adjust_bezier_g1(p, i);
                    }
                }
            }
            
            ImGui::Spacing();
            
            if (ImGui::Button("Add Point")) {
                u32 prev_p_id = get_stack_count(p)-1;
                v3 *np = (v3 *)stack_push(&p);
                *np = p[prev_p_id];
                
                if (prev_p_id % 3 == 0) {
                    adjust_bezier_c1(p, prev_p_id-1);
                }
                
            }
        }
        
        mat4 *transform = gl.transforms+entity->m_info.trans_id;
        update_transform(&gl, transform, entity->pos, entity->rot, entity->scale);
        
        ImGui::End();
        
        update_camera(&camera);
        
        //render_framebuffer(&gl);
        
        ImGui::Render();
        i32 w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGuiIO io = ImGui::GetIO();
        
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        stack_clear(gl.cmds);
        stack_clear(gl.verts);
        stack_clear(gl.uvs);
        stack_clear(gl.colors);
        stack_clear(gl.norms);
        stack_clear(gl.indices);
        
        f64 current_time = glfwGetTime();
        f64 delta = current_time - last_time;
        frame_count++;
        
        if (delta >= 1.0) {
            f64 fps = frame_count/delta;
            
            LOG("FPS: %f", fps);
            
            frame_count = 0;
            last_time = current_time;
            
        }
    }
    
}