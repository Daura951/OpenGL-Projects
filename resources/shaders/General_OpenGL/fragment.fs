#version 330

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 anormal;

out vec4 FragColor;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 camPos;

uniform Material material;
uniform DirectionalLight dirLight;

#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];


vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDirection = normalize(-light.direction);

    float diffStrength = max(dot(lightDirection, normal), 0.0);
    vec3 diffuse = light.diffuse * diffStrength * vec3(texture(material.diffuse, texCoord));

    vec3 reflectVec = reflect(-lightDirection, normal);
    float specStrength = pow(max(dot(reflectVec, viewDir), 0.0), material.shininess);
    vec3 specular = light.specular * specStrength * vec3(texture(material.specular, texCoord));
    
    vec3 ambient  = light.ambient * vec3(texture(material.diffuse, texCoord));

    return(ambient+diffuse+specular);
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDirection = normalize(light.position - fragPos);

    float diffStrength = max(dot(lightDirection, normal), 0.0);
    vec3 diffuse = light.diffuse * diffStrength * vec3(texture(material.diffuse, texCoord));

    vec3 reflectVec = reflect(-lightDirection, normal);
    float specStrength = pow(max(dot(reflectVec, viewDir), 0.0), material.shininess);
    vec3 specular = light.specular * specStrength * vec3(texture(material.specular, texCoord));

    vec3 ambient  = light.ambient* vec3(texture(material.diffuse, texCoord));

    float distance = length(light.position- fragPos);
    float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * distance * distance));
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return(ambient+diffuse+specular);

}

void main()
{
    vec3 normal= normalize(anormal);
    vec3 viewDir = normalize(camPos - fragPos);

    vec3 resultLight = calculateDirectionalLight(dirLight, normal, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) 
    {
        resultLight+=calculatePointLight(pointLights[i], normal, fragPos, viewDir);
    }

    FragColor = vec4(resultLight, 1.0);
}