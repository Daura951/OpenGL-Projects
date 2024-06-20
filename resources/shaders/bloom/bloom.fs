#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D hdrBuffer;
uniform sampler2D bloomBuffer;
uniform float exposure;
uniform bool isBloom;

void main()
{

    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, texCoord).rgb;
    vec3 bloomColor = texture(bloomBuffer, texCoord).rgb;
    hdrColor+=isBloom ?  bloomColor : 0.0; //simple hdr conversion, we just add bloom to hdr

    vec3 mappedLDR = vec3(1.0) -exp(-hdrColor*exposure);
    mappedLDR = pow(mappedLDR, vec3(1.0/gamma));
    FragColor = vec4(mappedLDR, 1.0);
}