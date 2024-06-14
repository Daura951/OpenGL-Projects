#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;

layout(std140) uniform Matricies
{
    mat4 projectionMat;
    mat4 viewMat;

};

uniform mat4 modelMat;

void main() {

    gl_Position = projectionMat * viewMat  * modelMat * vec4(aPos, 1.0);
    texCoord = aTexCoord;
}