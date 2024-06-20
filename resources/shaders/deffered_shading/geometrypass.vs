#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 texCoord;
out vec3 fragPos;
out vec3 normal;


uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelMat;

void main()
{
    fragPos = (modelMat * vec4(aPos, 1.0)).xyz; //make sure fragPos is in world coords
    texCoord = aTexCoords;
    normal = normalize((transpose(inverse(mat3(modelMat)))) * aNormal); //make sure normal is in world coords

    gl_Position = projectionMat * viewMat * modelMat * vec4(aPos, 1.0);
}