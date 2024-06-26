#version 330 core

out float FragColor;

in vec2 texCoord;

uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gNoise;

uniform vec3 samples[64];
uniform mat4 projectionMat;

const vec2 noiseScale = vec2(800.0/4.0, 800.0/4.0);

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;


void main()
{
    vec3 fragPos = texture(gPos, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 randomNoiseVec = texture(gNoise, texCoord*noiseScale).xyz; //represents out random tangent, scale by noiseScale so that rotation is more pronounced

    //since we are making our samples in tangent space (z faces up) we need to convert samples to view space
    vec3 tangent = normalize(randomNoiseVec - normal * dot(randomNoiseVec, normal)); //project the randomNoise to axis orthogonal to normal (x axis of tangent space, remove nromal component)
    vec3 biTangent = cross(normal, tangent); //get axis perpendicular to normal and tangent
    mat3 TBN = mat3(tangent,biTangent,normal); //create TBN. Since we based it on the normal and the normal is in view space, no need to multiply by a viewMat

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; i++)
    {
        vec3 samplePos = TBN * samples[i]; //we have our random sample points in tangent space, we convert them to view space
        samplePos = fragPos+samplePos*radius; //offset it by the fragment position in order to get the sample withing that fragments hemisphere

        vec4 offset = vec4(samplePos, 1.0);
        offset = projectionMat * offset; //view to clip space
        offset.xyz /=offset.w; //perspective divide
        offset.xyz = offset.xyz*0.5+0.5; //transform to 0-1

        float smapleDepth = texture(gPos, offset.xy).z; //grab the z value from the gPos

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - smapleDepth)); //smooth out in case of surface

        // Calculate occlusion based on depth comparison:
        // If the depth of the sampled point (`sampleDepth`) is greater than or equal to
        // the depth of the current sample point as viewed from the camera (`samplePos.z + bias`),
        // it indicates that the sampled point is farther away or at the same distance as
        // the current sample point in view space. This suggests less ambient light reaches
        // the current point, so we increase occlusion and reduce ambient light contribution.
        // basically if the depth in screen space is greater than in view space, it implies the sample is being occlueded
        occlusion+=(smapleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
    }

    occlusion = 1.0 - (occlusion/kernelSize); // as occlusion gets bigger, occlusion / kernelSize approaches 1. 1-1 = 0 => darkness
    FragColor = occlusion;
}