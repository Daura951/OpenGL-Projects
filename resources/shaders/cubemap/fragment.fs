#version 330 core


out vec4 FragColor;

in vec3 position;
in vec3 Normal;

uniform vec3 camPos;
uniform samplerCube skybox;

void main()
{
    vec3 I = normalize(position - camPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}