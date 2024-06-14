#version 330 core

out vec4 FragColor;

in vec2 texCoord;


uniform sampler2D texture1;

float near = 0.1;
float far = 100.0;

float linearizeDepth(float depth)
{
    float z  = depth * 2.0 - 1.0; //NDC
    return (2.0 * near * far) / (far+near - z * (far-near)); //reverse matrix projection to return to view space
}

void main()
{
    float depth = linearizeDepth(gl_FragCoord.z) / far;



    FragColor = texture(texture1, texCoord);
}