#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 texCoord;

uniform mat4 projectionMat;
uniform mat4 viewMat;

void main()
{
    texCoord = aPos;

    vec4 pos = projectionMat * viewMat * vec4(aPos, 1.0);
    gl_Position = pos.xyww; 
    //That way the z axis is always 1 so we can set depthFunc to GL_LEQUAL so that it'll only render when stuff is not in front of skybox
    //We do this because the last thing we want to render is the skybox!
}