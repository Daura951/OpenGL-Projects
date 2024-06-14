#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 3) in mat4 aInstanceMat;

out vec2 texCoord;

uniform mat4 viewMat;
uniform mat4 projectionMat;

void main() {
    gl_Position = projectionMat * viewMat * aInstanceMat * vec4(aPos, 1.0);
    texCoord = aTexCoord;
}