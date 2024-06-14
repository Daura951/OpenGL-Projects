#version 330 core

out vec4 FragColor;

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D diffuseTex;
uniform bool isBlinn;

void main()
{
    vec3 color = texture(diffuseTex, fs_in.texCoord).rgb;

    vec3 lightDirection = normalize(lightPos - fs_in.fragPos);
    vec3 viewDirection = normalize(viewPos - fs_in.fragPos);

    vec3 ambient = 0.05 * color;

    float diff=  max(dot(lightDirection, fs_in.normal), 0.0);

    vec3 diffuse = diff * color;

    float spec = 0.0;

    if(isBlinn)
    {
        vec3 halfwayVec = normalize(lightDirection+viewDirection);
        spec = pow(max(dot(halfwayVec, fs_in.normal), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDirection = reflect(-lightDirection, fs_in.normal);
        spec  =pow(max(dot(reflectDirection, viewDirection), 0.0), 8.0);
    }

    vec3 specular = spec * vec3(0.3);

    FragColor = vec4(ambient+diffuse+specular, 1.0);
}