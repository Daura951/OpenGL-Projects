#version 330 core

out vec4 FragColor;

in vec3 localPos;

uniform samplerCube cubeMap;


void main()
{
    //vec3 envColor = texture(cubeMap, localPos).rgb;
    vec3 envColor = textureLod(cubeMap, localPos, 1.2).rgb;

    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

    FragColor = vec4(envColor, 1.0);
}