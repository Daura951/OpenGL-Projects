#version 330 core

out float FragColor;

in vec2 texCoord;

uniform sampler2D ssaoTex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));

    float result=  0.0;

    //loop 16 times, gather 16 samples, and average them per fragment
    for(int i = -2; i < 2; i++)
    {
        for(int j = -2; j < 2; j++)
        {
            vec2 offset= vec2(float(i), float(j)) * texelSize;
            result+=texture(ssaoTex, texCoord+offset).r;
        }
    }
    FragColor=result/(4.0 * 4.0);
}