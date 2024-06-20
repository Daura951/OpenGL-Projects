#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 viewMat;
uniform mat4 projectionMat;

out VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} vs_out;

void main()
{
    vs_out.fragPos = aPos;
    vs_out.normal = normalize(aNormal);
    vs_out.texCoord = aTexCoord;
    gl_Position = projectionMat * viewMat * vec4(aPos, 1.0);
}