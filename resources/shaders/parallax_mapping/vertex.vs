#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location=4) in vec3 aBiTangent;

uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelMat;

uniform vec3 lightPos;
uniform vec3 viewPos;

out VS_OUT
{
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentLightPos;
    vec3 tangentFragPos;
    vec2 texCoord;
} vs_out;

void main()
{
    vs_out.fragPos = vec3(modelMat *  vec4(aPos,1.0));
    vs_out.texCoord = aTexCoord;

    vec3 T  = normalize(mat3(modelMat) * aTangent); //transform the tangent vectors out of tangent space
    vec3 B = normalize(mat3(modelMat) * aBiTangent); //transform the bitangent vectors out of tangent space
    vec3 N = normalize(mat3(modelMat) * aNormal); //transform the normal vectors out of tangent space
    mat3 TBN = transpose(mat3(T,B,N)); //Create an inverse of TBN matrix so we can cahnge world vectors into tangt space

    vs_out.tangentLightPos = TBN * lightPos; //transform light pos into tangent space
    vs_out.tangentViewPos  = TBN * viewPos; //transform view posinto tangent space
    vs_out.tangentFragPos  = TBN * vs_out.fragPos; //transform frag pos into tangent space

    gl_Position = projectionMat * viewMat * modelMat* vec4(aPos, 1.0);
}