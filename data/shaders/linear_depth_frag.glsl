#version 330 core

out vec4 frag_color;

uniform float near;
uniform float far;

float linearize_depth(float depth) {
    float ndc = depth * 2.0 - 1.0;
    return (2.0 * near * far)/(far + near - ndc * (far - near));
}

void main()  {
    float depth = linearize_depth(gl_FragCoord.z)/far;
    frag_color = vec4(vec3(depth), 1.0);
}