#version 330 core

out vec4 FragColor;

struct Light
{
    vec3 position;
    vec3 color;
};

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

uniform Light lights[4];
uniform sampler2D diffuseTex;
uniform vec3 viewPos;

void main()
{
    vec3 color = texture(diffuseTex, fs_in.texCoord).rgb;
    vec3 ambient = 0.0 * color;

    vec3 finalLighting = vec3(0.0);

    for(int i = 0; i < 4; i++)
    {
        vec3 lightDir = normalize(lights[i].position- fs_in.fragPos);

        float diff = max(dot(lightDir, fs_in.normal), 0.0);
        vec3 diffuse = lights[i].color * color * diff;

        vec3 result = diffuse;
        float dist = length(fs_in.fragPos - lights[i].position);
        result *= 1.0 / (dist*dist);
        finalLighting+=result;
    }

    FragColor = vec4(finalLighting, 1.0);
}