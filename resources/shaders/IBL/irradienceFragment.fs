#version 330 core

out vec4 FragColor;

in vec3 localPos;

uniform samplerCube cubeMap;

const float PI = 3.14159265359;
void main()
{
    vec3 normal =  normalize(localPos);

    vec3 irradience = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float numOfSamples = 0.0;

    //reflectance requires us to take the derivative of solid angle...that is hard!
    //Instead, since we are on a sphere we can use its polar equivalents, theta phi
    //This splits up the integral of diffuse into 2 integrals. the integral of polar azimuth phi
    //between 0 and 2pi and the integral of the zenith theta between 0 and 1/2 pi 
    for(float phi = 0.0; phi < 2.0 * PI; phi+=sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta+=sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)); //convert from spherical to cartesian (in tangent space now!)

            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; //get out of tangent space and into world space oriented around normal

            irradience+=texture(cubeMap, sampleVec).rgb * cos(theta) * sin(theta); //texture! the trig stuff helps with light
            numOfSamples++;
        }
    }
    irradience = PI * irradience * (1.0 / float(numOfSamples)); //at the end, we apply this and finish convolution

    FragColor = vec4(irradience, 1.0);
}