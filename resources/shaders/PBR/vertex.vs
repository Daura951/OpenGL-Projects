#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;


out vec2 texCoord;
out vec3 normal;
out vec3 worldPos;

uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelMat;

void main()
{
    texCoord = aTexCoord;
    worldPos = vec3(modelMat * vec4(aPos, 1.0));
    normal = normalize(transpose(inverse(mat3(modelMat))) * aNormal);
    gl_Position  =  projectionMat * viewMat * modelMat * vec4(aPos, 1.0);
}
