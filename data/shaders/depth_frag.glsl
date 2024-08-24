#version 330 core

out vec4 frag_color;

void main()  {
    frag_color = vec4(vec3(gl_FragCoord.z), 1.0);
}