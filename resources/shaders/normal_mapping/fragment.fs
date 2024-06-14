#version 330 core

out vec4 FragColor;

in VS_OUT
{
    vec3 fragPos;
    mat3 TBN;
    vec3 tangentViewPos;
    vec3 tangentLightPos;
    vec3 tangentFragPos;
    vec2 texCoord;
    vec3 normal;
} fs_in;

uniform sampler2D diffuseTex;
uniform sampler2D normalMap;
uniform bool isBlinn;

void main()
{
    vec3 color = texture(diffuseTex, fs_in.texCoord).rgb;

    //vec3 normal = fs_in.normal;
    vec3 normal = texture(normalMap, fs_in.texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0); //no need to convert to model spave since everyone else is in tangent space. Just grab from map

    vec3 lightDirection = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos); //notice that we are in tangent space here!
    vec3 viewDirection = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);

    //normal blinn-phong

    vec3 ambient = color * 0.1;

    float diff=  max(dot(lightDirection, normal), 0.0);

    vec3 diffuse = diff * color;


    vec3 halfwayVec = normalize(lightDirection+viewDirection);
    float spec = pow(max(dot(halfwayVec, normal), 0.0), 32.0);



    vec3 specular = spec * vec3(0.3);

    FragColor = vec4(ambient + diffuse+specular, 1.0);
}