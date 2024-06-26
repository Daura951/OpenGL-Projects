#version 330 core

out vec2 FragColor;
in vec2 texCoord;

const float PI = 3.14159265359;

//given a uniformly quasi random monte carlo sample vector (quasi since sample is from specular lobe)
//get it our of spherical and into world, this becomes an importance sample
vec3 importanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0  -cosTheta*cosTheta);

    //convert from spherical to cartesian system
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    //get out of tangent space and into world space
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0,0.0,1.0) : vec3(1.0,0.0,0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 biTangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + biTangent * H.y + N * H.z;
    return normalize(sampleVec);
}

//http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html explains why this is. Its simplified van der corpt
//converts number into a low descrepancy sequence (gives us uniformly distributed quasi-random samples)
float radicalInverseVanDerCorpt(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//helps with distributing samples fromv an der corpt
vec2 hammersley(uint i,  uint N)
{
    return vec2(float(i)/ float(N), radicalInverseVanDerCorpt(i));
}

//just like before, it helps with figuring out how much overshadow exists in microfacets
float geometrySchlickGGX(vec3 normal, vec3 vec, float roughness)
{
    float k = (roughness*roughness) / 2.0; //notice k is different, we are using the IBL version of k


    float NDotV = max(dot(normal, vec), 0.0); //same as before!

    return NDotV / (NDotV * (1.0 - k) + k);
}

//2nd part of geometry function
float geometrySmith(vec3 normal, vec3 lightDir, vec3 viewDir, float roughness)
{
    float ggx1 = geometrySchlickGGX(normal, viewDir, roughness);
    float ggx2 = geometrySchlickGGX(normal, lightDir, roughness);

    return ggx1*ggx2;
}

//We need to integrate brdf over hemisphere omega. We do this via monte carlo sampling
//We can then store this into a texture where horizontal is dot(normal, viewDir) and vertical is roughness
vec2 integrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1 -NdotV * -NdotV); //reconstruct view direction based on quad texCoord
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0; //define some accumulators for brdf
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;

    for(uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 xi =hammersley(i, SAMPLE_COUNT); //generate some uniformly distributed quasi-monte carlo sample vectors (quasi becuase of specular lobe)
        vec3 H = importanceSampleGGX(xi, N, roughness); //convert them from spherical to world space and importance sample
        vec3 L = normalize(2.0 * dot(V, H) * H - V); //reflect the view direction around microfacet sample to get light direction

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0) //if the sample contributes to brdf
        {
            float G = geometrySmith(N, L, V, roughness); //get the amount of overshadow in quasi-random sample
            float G_Vis = (G * VdotH) / (NdotH * NdotV); //account for visibility
            float Fc = pow(1.0 - VdotH, 5.0); //compute fresnel 

            A += (1.0 - Fc) * G_Vis; //accumulate non-fresnel (dot(norma, viewDirection))
            B += Fc * G_Vis; //accumulate fresnel (roughness)
        }
    }
    
    A /= float(SAMPLE_COUNT); //average it all out
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

void main()
{
  vec2 integratedBRDF = integrateBRDF(texCoord.x, texCoord.y);
    FragColor = integratedBRDF;
}