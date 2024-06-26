#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 worldPos;

uniform vec3 viewPos;

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform samplerCube irradienceMap;
uniform samplerCube preFilterMap;
uniform sampler2D brdfLUT;

struct Light {
    vec3 position;
    vec3 color;
};

const int LIGHT_AMT = 4;
uniform Light lights[LIGHT_AMT];

const float PI = 3.14159265359;

//fresnel is the ratio between how much a surface reflects and refracts light
vec3 fresnel(vec3 halfwayVec, vec3 viewDirection, vec3 baseReflectivity)
{
    float cosTheta = max(dot(halfwayVec, viewDirection), 0.0);

    return baseReflectivity + (1 - baseReflectivity) * pow(clamp(1.0 - cosTheta, 0.0, 1.0),5.0); //fresnel from notes, clamp just helps against black spots
}

//adjusted fresnel for roughness! Notive the difference here
vec3 fresnelRoughness(vec3 normal, vec3 viewDirection, vec3 baseReflectivity, float roughness)
{
    float cosTheta = max(dot(normal, viewDirection), 0.0);
    return baseReflectivity + (max(vec3(1.0-roughness), baseReflectivity) - baseReflectivity) * pow(clamp(1.0 - cosTheta, 0.0, 1.0),5.0);
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
    vec3 viewDirection  = normalize(viewPos - worldPos);
    
    vec3 totalRadience = vec3(0.0);

    vec3 baseReflectivity = vec3(0.04); //default for nonmetals
    baseReflectivity = mix(baseReflectivity, albedo, metallic); //if a surface is completely metallic, we use the albedo

    for(int i = 0; i < LIGHT_AMT; i++)
    {
        vec3 lightDirection = normalize(lights[i].position - worldPos);
        vec3 halfwayVec  = normalize(lightDirection + viewDirection);
        
        float distance = length(lights[i].position - worldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radience = lights[i].color * attenuation; //the color essentially represents flux! Remember the wavelength thing and how it correlates to flux

        vec3 fres = fresnel(halfwayVec, viewDirection, baseReflectivity);

        float NDF = trowbridgeReitzGGX(normal, halfwayVec, roughness);
        float GFunction = geometrySmith(normal, lightDirection, viewDirection, roughness);

        vec3 DFG = NDF * GFunction * fres;
        float brdfNormalizer = 4.0 * max(dot(normal, viewDirection), 0.0) * max(dot(normal, lightDirection), 0.0) + 0.0001;
        vec3 specular = DFG / brdfNormalizer; //gives us the specular component of brdf

        vec3 ks = fres; //remember what fresnel does!
        vec3 kd = vec3(1.0) - ks; //get the amount of diffuse

        kd *=1.0 - metallic; //get rid of diffuse if we are metallic!

        float NDotL = max(dot(normal, lightDirection), 0.0); //determine intensity just like in diffuse lighting
        totalRadience+= (kd * albedo/PI + specular) * radience * NDotL; //complete radience equation
    }

    vec3 ks = fresnelRoughness(normal, viewDirection, baseReflectivity, roughness); //now we need the fresnel under roughess for diffuse light
    vec3 kd = vec3(1.0) - ks; //get diffuse ratio
    kd *=1.0 - metallic; //eliminate if metallic
    vec3 irradience = texture(irradienceMap, normal).rgb; //sample from the irradience map based on the normal
    vec3 diffuse = irradience * albedo; //calculate diffuse color

    //Since specular is a reflection based thing, we need to get the reflection relative to the camers
    vec3 reflectVec = reflect(-viewDirection, normal);

    const float MAX_REFLECTION_LOD = 4.0; //needed so that we dont go over our mipmap amounts
    vec3 preFilerColor = textureLod(preFilterMap, reflectVec, roughness*MAX_REFLECTION_LOD).rgb; //prefilter map gets the basic specular parts of specular integral based on specular lobe

    //get the split sum of brdf integal from texture. This texture is the relationship between angle (like in diffuse lighting) and roughness
    vec2 envBRDF = texture(brdfLUT, vec2(max(dot(normal, viewDirection), 0.0), roughness)).rg; 
    vec3 specular = preFilerColor * (ks * envBRDF.x + envBRDF.y);

    vec3 ambient = (kd * diffuse + specular) * ao; //adjust diffuse based on fresnel ratio, and add ambeint occlusion
    vec3 color = ambient+totalRadience; //apply to final color

    color /= color + vec3(1.0); //apply hdr
    color = pow(color, vec3(1.0/2.2)); //apply gamma correction
    
    FragColor = vec4(color, 1.0); //output the color
}