#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNomral;

out vec3 position;
out vec3 Normal;


uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main() {

    Normal = vec3(transpose(inverse(modelMat))) * aNomral;
    position = vec3(modelMat * vec4(aPos, 1.0));
    gl_Position = projectionMat * viewMat  * vec4(position, 1.0);
}