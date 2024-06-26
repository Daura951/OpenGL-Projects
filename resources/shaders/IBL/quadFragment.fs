#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D image;

void main()
{
    FragColor = vec4(texture(image, texCoord).rgb, 1.0);
}