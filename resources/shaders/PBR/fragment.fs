#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 worldPos;

uniform vec3 viewPos;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D normalMap;


struct Light {
    vec3 position;
    vec3 color;
};

const int LIGHT_AMT = 4;
uniform Light lights[LIGHT_AMT];

const float PI = 3.14159265359;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, texCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(worldPos);
    vec3 Q2  = dFdy(worldPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

//fresnel is the ratio between how much a surface reflects and refracts light
vec3 fresnel(vec3 halfwayVec, vec3 viewDirection, vec3 baseReflectivity)
{
    float cosTheta = max(dot(halfwayVec, viewDirection), 0.0);

    return baseReflectivity + (1 - baseReflectivity) * pow(clamp(1.0 - cosTheta, 0.0, 1.0),5.0); //fresnel from notes, clamp just helps against black spots
}

//This function determines how much overshadow exists in the microfacets
float geometrySchlickGGX(vec3 normal, vec3 vec, float roughness)
{
    float r  = (roughness+1.0);
    float k = (r*r) / 8.0;

    float NDotV = max(dot(normal, vec), 0.0);

    return NDotV / (NDotV * (1.0 - k) + k);
}

//2nd part of geometry function
float geometrySmith(vec3 normal, vec3 lightDir, vec3 viewDir, float roughness)
{
    float ggx1 = geometrySchlickGGX(normal, viewDir, roughness);
    float ggx2 = geometrySchlickGGX(normal, lightDir, roughness);

    return ggx1*ggx2;
}

//Determines how many microfacets are aligned with the halfway vec, as roughness increases, a larger area of microfacets get aligned
float trowbridgeReitzGGX(vec3 normal, vec3 halfwayVec, float roughness)
{
    float a = roughness*roughness;
    float aSquared = a*a;

    float NDotHSquared=  max(dot(normal, halfwayVec), 0.0) * max(dot(normal, halfwayVec), 0.0);

    return aSquared / (PI * pow((NDotHSquared * (aSquared - 1.0) + 1.0), 2.0));
}

void main()
{
    vec3 albedo = texture(albedoMap, texCoord).rgb;
    float metallic = texture(metallicMap, texCoord).r;
    float roughness = texture(roughnessMap, texCoord).r;
    float ao = texture(aoMap, texCoord).r;

    vec3 Normal = getNormalFromMap();
    vec3 viewDirection  = normalize(viewPos - worldPos);
    
    vec3 totalRadience = vec3(0.0);

    for(int i = 0; i < LIGHT_AMT; i++)
    {
        vec3 lightDirection = normalize(lights[i].position - worldPos);
        vec3 halfwayVec  = normalize(lightDirection + viewDirection);
        
        float distance = length(lights[i].position - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radience = lights[i].color * attenuation; //the color essentially represents flux! Remember the wavelength thing and how it correlates to flux


        vec3 baseReflectivity = vec3(0.04); //default for nonmetals
        baseReflectivity = mix(baseReflectivity, albedo, metallic); //if a surface is completely metallic, we use the albedo
        vec3 fres = fresnel(halfwayVec, viewDirection, baseReflectivity);

        float NDF = trowbridgeReitzGGX(Normal, halfwayVec, roughness);
        float GFunction = geometrySmith(Normal, lightDirection, viewDirection, roughness);

        vec3 DFG = NDF * GFunction * fres;
        float brdfNormalizer = 4.0 * max(dot(Normal, viewDirection), 0.0) * max(dot(Normal, lightDirection), 0.0) + 0.0001;
        vec3 specular = DFG / brdfNormalizer; //gives us the specular component of brdf

        vec3 ks = fres; //remember what fresnel does!
        vec3 kd = vec3(1.0) - ks; //get the amount of diffuse

        kd *=1.0 - metallic; //get rid of diffuse if we are metallic!

        float NDotL = max(dot(Normal, lightDirection), 0.0); //determine intensity just like in diffuse lighting
        totalRadience+= (kd * albedo/PI + specular) * radience * NDotL; //complete radience equation
    }
    vec3 ambient = vec3(0.03) * albedo * ao; //determine ambient based on ao
    vec3 color = ambient+totalRadience; //apply everything

    color /= color + vec3(1.0); //apply hdr
    color = pow(color, vec3(1.0/2.2)); //apply gamma correction
    
    FragColor = vec4(color, 1.0); //output the color
}