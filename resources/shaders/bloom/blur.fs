#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D image;

uniform bool isHorizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 texelSize = 1.0 / textureSize(image, 0); //grab the suze if  a texel
    vec3 result  =texture(image, texCoord).rgb * weight[0]; //initalize result

    if(isHorizontal)
    {
        for(int i = 1; i < 5; i++) //do convolution on horizontal component
        {
            result+=texture(image, texCoord + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
            result+=texture(image, texCoord + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; i++) //do convolution on vertical component
        {
            result+=texture(image, texCoord + vec2(0.0, texelSize.y * i)).rgb * weight[i];
            result+=texture(image, texCoord + vec2(0.0, texelSize.y * i)).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
    
}