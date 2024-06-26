#version 330 core

out vec4 FragColor;

in vec2 texCoord;

struct Light
{
    vec3 position;
    vec3 color;

    float radius;
};


uniform Light light;
uniform vec3 viewPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

void main()
{
    vec3 fragPos = texture(gPosition, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 albedo = texture(gAlbedoSpec, texCoord).rgb;
    float specular = texture(gAlbedoSpec, texCoord).a;
    float ambientOcclision = texture(ssao, texCoord).r;

    vec3 ambient = vec3(0.3 * albedo * ambientOcclision); //this is where he comes into play
    vec3 lighting = ambient;

    vec3 viewDir = normalize(-fragPos);
    vec3 lightDir = normalize(light.position - fragPos);


    float distance = length(light.position - fragPos);

    float diff = max(dot(lightDir, normal), 0.0);

    vec3 diffuse = albedo * light.color * diff;

    //float attenuation = 1.0 / (1.0 + (0.7 * distance) + (1.8 * (distance * distance)));

    lighting+=diffuse*1;
        
    

    FragColor = vec4(lighting, 1.0);
}