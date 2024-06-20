#version 330 core

out vec4 FragColor;

in vec2 texCoord;

struct Light
{
    vec3 position;
    vec3 color;

    float radius;
};

const int LIGHT_AMT = 32;
uniform Light lights[LIGHT_AMT];
uniform vec3 viewPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

void main()
{
    //lighting time. I decided to jsut do diffuse here. Sample what we need for lighting from the textures in g-buffer
    vec3 fragPos = texture(gPosition, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 albedo = texture(gAlbedoSpec, texCoord).rgb;
    float specular = texture(gAlbedoSpec, texCoord).a;

    vec3 lighting = albedo * 0.1; //ambient
    vec3 viewDir = normalize(viewPos - fragPos);


    for(int i = 0; i < LIGHT_AMT; i++)
    {
        float distance = length(lights[i].position - fragPos);

        if(distance < lights[i].radius) //if we are within radius of lighting, calculate lighting
        {
            vec3 lightDir = normalize(lights[i].position - fragPos);
            float diff = max(dot(lightDir, normal), 0.0);
            vec3 diffuse = albedo * lights[i].color * diff;

            float attenuation = 1.0 / (1.0 + (0.7 * distance) + (1.8 * (distance * distance)));

            lighting+=diffuse*attenuation;
        }
    }

    FragColor = vec4(lighting, 1.0);
}