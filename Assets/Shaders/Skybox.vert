#version 460

layout(location = 0) out vec3 texVec;

layout(set = 1, binding = 0) uniform  CameraBuffer{
	mat4 view;
	mat4 proj;
    mat4 projview;
    vec4 forward;
    vec4 right;
    vec4 up;
    vec4 reserved;
} SAGECamera;

//need fixed skybox verts
const vec2 verts[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(-1.0,1.0),
    vec2(-1.0,-1.0)
);

void main(){
    vec2 vert = verts[gl_VertexIndex];
    gl_Position = vec4(vert, 1.0, 1.0);
    texVec = normalize((SAGECamera.forward + vert.x * SAGECamera.right - vert.y * SAGECamera.up).xyz);
}