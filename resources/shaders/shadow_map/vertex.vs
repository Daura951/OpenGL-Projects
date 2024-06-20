#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;


out VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec4 fragPosLightSpace;
} vs_out;


uniform mat4 projectionMat;
uniform mat4 viewMat;
uniform mat4 modelMat;
uniform mat4 lightSpaceMat;

void main()
{
    vs_out.fragPos = vec3(modelMat * vec4(aPos, 1.0));
    vs_out.normal = normalize(transpose(inverse(mat3(modelMat))) * aNormal);
    vs_out.texCoord = aTexCoord;
    vs_out.fragPosLightSpace = lightSpaceMat * vec4(aPos, 1.0);
    gl_Position = projectionMat * viewMat * modelMat * vec4(aPos, 1.0);

}