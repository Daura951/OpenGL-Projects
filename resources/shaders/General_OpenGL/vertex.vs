#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;


out vec2 texCoord;
out vec3 anormal;
out vec3 fragPos;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main() {
    gl_Position =   projectionMat * viewMat * modelMat * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    texCoord = aTexCoord;
    anormal = mat3(transpose(inverse(modelMat))) * aNormal;
    fragPos = vec3(modelMat * vec4(aPos, 1.0));
}