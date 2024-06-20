#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main()
{

    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, texCoord).rgb;

    vec3 mappedLDR = vec3(1.0) -exp(-hdrColor*exposure);
    mappedLDR = pow(mappedLDR, vec3(1.0/gamma));
    FragColor = vec4(mappedLDR, 1.0);
}