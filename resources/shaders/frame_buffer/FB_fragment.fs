#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTex;

const float convOffset = 1.0/300.0;

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-convOffset,  convOffset), // top-left
        vec2( 0.0f,    convOffset), // top-center
        vec2( convOffset,  convOffset), // top-right
        vec2(-convOffset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( convOffset,  0.0f),   // center-right
        vec2(-convOffset, -convOffset), // bottom-left
        vec2( 0.0f,   -convOffset), // bottom-center
        vec2( convOffset, -convOffset)  // bottom-right    
    );

    float kernel[9] = float[](

        1, 0,  -1,
        2, 0,  -2,
        1, 0,  -1
    );

    vec3 sampleTex[9];

    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTex, texCoord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i  < 9; i++)
    {
        col+=sampleTex[i] * kernel[i];
    }
    FragColor = vec4(col, 1.0);
}