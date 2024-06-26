#version 330 core

out vec4 FragColor;

in vec3 localPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 sampleFromSphere(vec3 vec)
{
    vec2 uv = vec2(atan(vec.z, vec.x), asin(vec.y));
    uv *= invAtan;
    uv +=0.5;
    return uv;
}

void main()
{
    vec2 uv = sampleFromSphere(normalize(localPos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    FragColor = vec4(color, 1.0);
}