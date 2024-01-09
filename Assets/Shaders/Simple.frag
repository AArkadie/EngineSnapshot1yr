#version 460

layout(set = 2, binding = 0) uniform sampler2D SAGEDiffuseMap;

layout(location = 0) in vec4 inColor;

layout(location=0) out vec4 outColor;

void main(){
    outColor = texture(SAGEDiffuseMap, inColor.zw);
    if (outColor.w < 0.1) discard;
}