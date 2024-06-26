#version 330 core

layout(location=0) out vec3 gPos; //notice our layouts! We wire our color buffers to go to those! (COLOR_ATTACHMENT0 is location 0)
layout(location=1) out vec3 gNormal;
layout(location=2) out vec4 gAlbedoSpec;

in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

void main()
{
    //apply textures
    gPos = fragPos;
    gNormal = normal;
    gAlbedoSpec.rgb = vec3(0.95); //just using this becuase I dont want texture. This is to show off lighting
}