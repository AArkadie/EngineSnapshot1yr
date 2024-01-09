#version 450

void main(){
    const vec3 posi[6] = vec3[6](
        vec3(-1.f, -1.f, 0.f),
        vec3( 1.f, -1.f, 0.f),
        vec3( 1.f,  1.f, 0.f),
        vec3( 1.f,  1.f, 0.f),
        vec3(-1.f,  1.f, 0.f),
        vec3(-1.f, -1.f, 0.f)
    );

    gl_Position = vec4(posi[gl_VertexIndex], 1.f);
}