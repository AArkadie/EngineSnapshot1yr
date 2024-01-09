#version 460

layout(location = 0) in vec4 inPosiTex;
layout(location = 0) out vec4 outVec;

 void main() {
    gl_Position = vec4(inPosiTex.xy, 0.0, 1.0);
    outVec = inPosiTex;
 }