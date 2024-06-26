#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D image;

void main()
{
    float occlusion = texture(image, texCoord).r;
    FragColor = vec4(vec3(occlusion,occlusion,occlusion), 1.0);
}