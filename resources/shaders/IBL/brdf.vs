#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexcoord;

out vec2 texCoord;

void main()
{
    texCoord = aTexcoord;
    gl_Position = vec4(aPos, 1.0);
}