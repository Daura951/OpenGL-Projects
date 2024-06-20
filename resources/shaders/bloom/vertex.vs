#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} vs_out;

uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelMat;
uniform bool invertNormals;

void main()
{
    vs_out.normal = normalize(invertNormals ?  -aNormal : aNormal);
    vs_out.fragPos = vec3(modelMat * vec4(aPos, 1.0));
    vs_out.texCoord = aTexCoord;

    gl_Position = projectionMat*viewMat*modelMat*vec4(aPos, 1.0);
}