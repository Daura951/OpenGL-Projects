#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;


out VS_OUT {
 vec2 texCoord;
}vs_out;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main() {
    gl_Position = projectionMat * viewMat * modelMat * vec4(aPos, 1.0);
    vs_out.texCoord = aTexCoord;
}