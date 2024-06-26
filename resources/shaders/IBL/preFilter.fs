#version 330 core

out vec4 FragColor;

in vec3 localPos;

uniform samplerCube cubeMap;
uniform float roughness;

const float PI = 3.14159265359;

float trowbridgeReitzGGX(vec3 normal, vec3 halfwayVec, float roughness)
{
    float a = roughness*roughness;
    float aSquared = a*a;

    float NDotHSquared=  max(dot(normal, halfwayVec), 0.0) * max(dot(normal, halfwayVec), 0.0);

    return aSquared / (PI * pow((NDotHSquared * (aSquared - 1.0) + 1.0), 2.0));
}

vec3 importanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0  -cosTheta*cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0,0.0,1.0) : vec3(1.0,0.0,0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 biTangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + biTangent * H.y + N * H.z;
    return normalize(sampleVec);
}

//http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html explains why this is. Its simplified van der corpt
//converts number into a low descrepancy sequence (gives us uniformly distributed quasi-monte carlo samples)
float radicalInverseVanDerCorpt(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
//helps with distributing samples from van der corpt
vec2 hammersley(uint i,  uint N)
{
    return vec2(float(i)/ float(N), radicalInverseVanDerCorpt(i));
}

void main()
{
    vec3 normal = normalize(localPos);
    vec3 R = normal;
    vec3 V = R;

    const uint SAMPLE_COUNT = 4096u;
    float totalWeight = 0.0;
    vec3 prefilterColor = vec3(0.0);
    for(uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 xi =hammersley(i, SAMPLE_COUNT); //generate quasi-monte carlo sample
        vec3 H = importanceSampleGGX(xi, normal, roughness); //transform them to world space and apply scattering based on roughness
        vec3 L = normalize(2.0 * dot(V, H) * H - V); //get light direction form view direction and microfacet halfway vector

        float NdotL = max(dot(normal, L), 0.0);

        if(NdotL > 0)
        {
            float D = trowbridgeReitzGGX(normal, H, roughness);
            float pdf = (D * max(dot(normal, H), 0.0) / (4.0 *  max(dot(normal, H), 0.0))+ 0.0001);
            
            float resolution = 512.0;
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            prefilterColor  += textureLod(cubeMap, L, mipLevel).rgb * NdotL;
            totalWeight+=NdotL;
        }
    }
    prefilterColor = prefilterColor / totalWeight;
    FragColor = vec4(prefilterColor, 1.0);

}