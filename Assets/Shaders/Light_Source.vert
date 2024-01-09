#version 460

layout (location = 0) in vec3 lightPos;//only need this but we'll consume all other standard vertex data for now.
layout (location = 1) in vec3 vNormal;//just so we don't have to make a new layout
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec4 vColor;
layout (location = 5) in vec2 vTexCo;

struct ObjectData{
    mat4 modelMat;
    mat4 normalMat;
    mat4 unused2;
    mat4 unused3;
};

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer{
    ObjectData objects[];
}SAGEObjectDataBuffer;

layout(set = 1, binding = 0) uniform  CameraBuffer{
	mat4 view;
	mat4 proj;
    mat4 projview;
    vec4 forward;
    vec4 right;
    vec4 up;
    vec4 reserved;
} SAGECamera;

layout( push_constant ) uniform constants
{
    uint mmIndex;
    uint texIndex;
    uint otherIndex;
    uint motherIndex;
} PushConstants;

void main(){
    mat4 modelMatrix = SAGEObjectDataBuffer.objects[PushConstants.mmIndex].modelMat;
    //we can use gl_BaseInstance to pass in an index from the draw call's "firstInstance" parameter
    //that way, we could avoid using push constants or some other descriptor to do it.
	mat4 transformMatrix = (SAGECamera.projview * modelMatrix);
	gl_Position = transformMatrix * vec4(lightPos, 1.0f);
}