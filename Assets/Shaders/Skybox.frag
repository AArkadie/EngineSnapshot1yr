#version 460

layout(location = 0) in vec3 texVec;
layout(set = 2, binding = 0) uniform samplerCube SAGESkybox;

layout(location = 0) out vec4 fragColor;

void main(){
    fragColor = texture(SAGESkybox,texVec);
}