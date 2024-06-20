#version 330 core

out vec4 FragColor;

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
} fs_in;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 viewPos;
uniform sampler2D diffuseTex;
uniform bool isGamma;


vec3 blinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{

    vec3 lightDirection = normalize(lightPos -  fragPos);
    vec3 viewDirection = normalize(viewPos - fragPos);
    vec3 halfwayVec = normalize(lightDirection+viewDirection);

    float diff=  max(dot(lightDirection, normal), 0.0);

    vec3 diffuse = diff * lightColor;

    float spec = pow(max(dot(halfwayVec, normal), 0.0), 32.0);
    
    vec3 specular = spec * lightColor;

    float maxDistance = 1.5;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (isGamma ? (distance*distance) : distance);

    specular*=attenuation;
    diffuse*=attenuation;

    return diffuse+specular;
}

void main()
{
    vec3 color = texture(diffuseTex, fs_in.texCoord).rgb;
    vec3 lighting = vec3(0.0);

    for(int i = 0; i < 4; i++)
    {
        lighting+=blinnPhong(fs_in.normal, fs_in.fragPos, lightPositions[i], lightColors[i]);
    }
    
    color*=lighting;

    if(isGamma)
    {
        color = pow(color, vec3(1.0/ 2.2));
    }
    FragColor = vec4(color, 1.0);
}