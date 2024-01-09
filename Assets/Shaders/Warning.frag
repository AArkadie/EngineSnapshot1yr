#version 450

layout(location = 0) out vec4 colo;

void main(){
    colo = gl_FragCoord;
}