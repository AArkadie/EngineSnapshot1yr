#version 460

struct ObjectData{
    mat4 modelMat;
    mat4 normalMat;
    mat4 unused2;
    mat4 unused3;
};

struct LightData{
    vec4 position;
    vec4 color;
    vec4 attenuationFactors;
    vec4 spotlightData;
};

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec4 vColor;
layout (location = 5) in vec2 vTexCo;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCo;
layout (location = 2) out vec3 positionInView;
layout (location = 3) out mat3 TBN;

//all object matrices
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

layout(set = 1, binding = 1) uniform APSceneData{
    vec4 fogColor;//w is for exponent [note: using to pass in camera position ATM, w is for specular shininess]
    vec4 fogDistance;// x min y max zw
    vec4 ambientColor;
    vec4 reserved;

    LightData lights[3];
} SAGESceneData;

layout (set = 2, binding = 2) uniform sampler2D SAGEBumpMap;

//push constants block
layout( push_constant ) uniform constants
{
    uint mmIndex;
    uint texIndex;
    uint otherIndex;
    uint motherIndex;
} PushConstants;

void main()
{    
    mat4 modelMatrix = SAGEObjectDataBuffer.objects[PushConstants.mmIndex].modelMat;
    mat3 normalMatrix = mat3(SAGEObjectDataBuffer.objects[PushConstants.mmIndex].normalMat);
    //we can use gl_BaseInstance to pass in an index from the draw call's "firstInstance" parameter
    //that way, we could avoid using push constants or some other descriptor to do it.  Important in bindless?
    vec3 T = normalize(mat3(SAGECamera.view) * normalMatrix * vTangent);
    vec3 B = normalize(mat3(SAGECamera.view) * normalMatrix * vBitangent);
    vec3 N = normalize(mat3(SAGECamera.view) * normalMatrix * vNormal);
    TBN = mat3(T, B, N);

	mat4 transformMatrix = (SAGECamera.projview * modelMatrix);
	gl_Position = transformMatrix * vec4(vPosition, 1.0f);
	outColor = vec4(vPosition,1.0) * vColor;
    positionInView = vec3(SAGECamera.view * modelMatrix * vec4(vPosition,1.0f));
    vec3 tangentPos = transpose(TBN) * positionInView;
    vec2 scaledView = normalize(-(tangentPos.xy)) * texture(SAGEBumpMap, vTexCo).x * 0.01;
    outTexCo = vTexCo + scaledView;
}