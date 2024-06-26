#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 projectionMat;
uniform mat4 viewMat;


void main()
{
    localPos = aPos;

    mat4 rotView = mat4(mat3(viewMat)); //remove translation because skybox
    vec4 clipPos = projectionMat * rotView * vec4(aPos, 1.0);

    gl_Position = clipPos.xyww;
}