#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D depthMap;
uniform float nearPlane;
uniform float farPlane;

float linearizeDepth(float dpeth)
{
    float z = dpeth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane+nearPlane-z * (farPlane-nearPlane));
}

void main()
{
    float depthVal = texture(depthMap, texCoord).r;
    FragColor = vec4(vec3(depthVal), 1.0); //ortho
}