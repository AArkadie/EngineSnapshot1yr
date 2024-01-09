#version 460

//structs
struct LightData{
    vec4 position;
    vec4 color;
    vec4 attenuationFactors;
    vec4 spotlightData;
};

//function forward declarations
vec3 lightClassic(LightData);
vec3 pbrPathway(LightData);

//shader input
layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 fragPosView;
layout (location = 3) in mat3 TBN;

layout (set = 2, binding = 0) uniform sampler2D SAGEDiffuseMap;
layout (set = 2, binding = 1) uniform sampler2D SAGESpecularMap;
//layout (set = 2, binding = 2) uniform sampler2D SAGEBumpMap; in vertex right meow
layout (set = 2, binding = 3) uniform sampler2D SAGEAlbedoMap;
layout (set = 2, binding = 4) uniform sampler2D SAGENormalMap;
layout (set = 2, binding = 5) uniform sampler2D SAGEReflectivityMap;
layout (set = 2, binding = 6) uniform sampler2D SAGEGlossinessMap;
layout (set = 2, binding = 7) uniform sampler2D SAGEAmbientOcclusionMap;
//layout (set = 2, binding = 8) uniform sampler2D SAGEOpacityMap;
//layout (set = 2, binding = 9) uniform sampler2D SAGERefractionMap;
//layout (set = 2, binding = 10) uniform sampler2D SAGEEmissiveMap;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 1) uniform APSceneData{
    vec4 fogColor;//w is for exponent [note: using to pass in camera position ATM, w is for specular shininess]
    vec4 fogDistance;// x min y max zw
    vec4 ambientColor;
    vec4 reserved;

    LightData lights[3];
} SAGESceneData;

void main()
{
    //color starts out with only ambient contrib
    vec3 resultColor =(SAGESceneData.ambientColor.xyz * SAGESceneData.ambientColor.w) * vec3(texture(SAGEDiffuseMap,texCoord));
    //then we start summing
    for(int i = 0; i < 3; i++){
        resultColor += lightClassic(SAGESceneData.lights[i]);
    }
    resultColor /= resultColor + vec3(1.0);
	//return color
	outFragColor = vec4(resultColor,1.0);
}

vec3 lightClassic(LightData light){
    vec3 diffuseContribution;
    vec3 specularContribution;
    vec3 dirToLight;//omega incoming
    vec3 lightColor = light.color.xyz * light.color.w;
    vec3 viewDirection = normalize(-fragPosView);//V
    vec3 normal = normalize(TBN * ((vec3(texture(SAGENormalMap, texCoord)) * 2.0) - 1.0));
    float dist = length(light.position.xyz - fragPosView);
    float attenuation;
    uint comper = uint(light.position.w + 0.1);
    ivec4 vomper = ivec4(light.spotlightData + vec4(0.5));
    
    if(comper == 0){
        //do direction light stuff
        dirToLight = normalize(-light.position.xyz);
        attenuation = 1.0;
    }
    else{
        dirToLight = normalize(light.position.xyz - fragPosView);
        attenuation = 1.0 / (
        light.attenuationFactors.x +
        (light.attenuationFactors.y * dist) +
        (light.attenuationFactors.z * (dist * dist))
    );
    }
    //do pointLight stuff
    vec3 halfDir = normalize (dirToLight + viewDirection);

    float diffuseStr = clamp(dot(normal, dirToLight),0.0, 1.0);//cos theta

    //vec3 reflection = reflect(-dirToLight, chosenormal); --used for regular phong, dotted with viewDirection below
    float specularStr = pow(clamp(dot(normal, halfDir), 0.0, 1.0), SAGESceneData.fogColor.w);//cos theta

    diffuseContribution = lightColor * diffuseStr * vec3(texture(SAGEDiffuseMap, texCoord));
    specularContribution = lightColor * specularStr * vec3(texture(SAGESpecularMap, texCoord));  

    diffuseContribution *= attenuation;
    specularContribution *= attenuation;

    if(vomper != ivec4(0)){
        //do spotlight stuff
        float theta = dot(dirToLight, normalize(-light.spotlightData.xyz));
        if(theta < light.spotlightData.w){
            diffuseContribution = vec3(0.0);
            specularContribution = vec3(0.0);
        }
        else{
        float epsilon = light.attenuationFactors.w - light.spotlightData.w;
        float intensity = clamp((theta - light.spotlightData.w)/epsilon, 0.0, 1.0);
        diffuseContribution *= intensity;
        specularContribution *= intensity;
        }
    }

    return diffuseContribution + specularContribution;
}

vec3 pbrPathway(LightData light){
    const float PI = 3.14159265359;
    uint comper = uint(light.position.w + 0.1);
    ivec4 vomper = ivec4(light.spotlightData + vec4(0.5));
    vec3 mapColor = texture(SAGEAlbedoMap, texCoord).xyz;
    float metalness = texture(SAGEReflectivityMap, texCoord).x;
    float roughness = 1.0 - texture(SAGEGlossinessMap, texCoord).x;
    vec3 N = normalize(TBN * ((vec3(texture(SAGENormalMap, texCoord)) * 2.0) - 1.0));
    vec3 V = normalize(-fragPosView);
    vec3 L;
    float dist = length(light.position.xyz - fragPosView);
    float attenuation;
    if(comper ==  0){
        L = normalize(-light.position.xyz);
        attenuation = 1.0;
    }
    else{
        L = normalize(light.position.xyz - fragPosView);
        attenuation = 1.0 /(
         light.attenuationFactors.x +
        (light.attenuationFactors.y) * dist +
        (light.attenuationFactors.z * (dist*dist))
    );
    }
    vec3 H = normalize(V + L);
    float cosTheta = max(dot(H,V),0.0);
    vec3 radiance = (light.color.xyz * light.color.w) * attenuation;
    vec3 F0 = mix(vec3(0.4), mapColor, metalness);
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0,1.0), 5.0);
    float rough4 = pow(roughness, 4);
    float NdotH = max(dot(N,H),0.0);
    float NdotH2 = NdotH * NdotH;
    float domo = NdotH2 * (rough4 - 1.0) + 1.0;
    float NDF = rough4 / (PI * domo * domo);
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float NdotV = max(dot(N,V),0.0);
    float NdotL = max(dot(N,L),0.0);
    float G = (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));
    vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V),0.0) * max(dot(N, L),0.0) + 0.0001);
    vec3 diffuse = (vec3(1.0) - F) * (1.0 - metalness);
        if(vomper != ivec4(0)){
        //do spotlight stuff
        float theta = dot(L, normalize(-light.spotlightData.xyz));
        if(theta < light.spotlightData.w){
            radiance = vec3(0.0);
        }
        else{
        float epsilon = light.attenuationFactors.w - light.spotlightData.w;
        float intensity = clamp((theta - light.spotlightData.w)/epsilon, 0.0, 1.0);
        radiance *= intensity;
        }
    }
    return (diffuse * mapColor / PI + specular) * radiance * NdotL;
}