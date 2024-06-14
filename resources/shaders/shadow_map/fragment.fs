#version 330 core
out vec4 FragColor;

in VS_OUT
{
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec4 fragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTex;
uniform sampler2D depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projectedCoords = fragPosLightSpace.xyz/  fragPosLightSpace.w; //perspective divide [-1, 1]
    projectedCoords = projectedCoords * 0.5 + 0.5; //[0,1]

    float closestDepth = texture(depthMap, projectedCoords.xy).r; //Textures are in NDC. By making the coords NDC, we can pull from the dpeth map
    float currentDepth  = projectedCoords.z; //get the current Z position of fragment
    float bias  =0.005; //bias in order to stop waves of shadows thanks to sample de-resolution

    float shadow = 0.0;

    //PCF to help with less rigid shadows
    vec2 texelSize = 1.0 / textureSize(depthMap, 0); //this allows us to get the size of a single texel
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            float pcfDepth = texture(depthMap, projectedCoords.xy + vec2(i,j) * texelSize).r; //perform pcf
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0; //check and see if we are a shadow or not (1= shadow, 0=not shadow)
        }
    }
    shadow/=10.0; //average out the shadow from pcf

    if(projectedCoords.z > 1.0) //helps avoid weird shadows on plane
    {
        shadow = 0.0;
    }


    return shadow;


}

//usual blinn phong lighting model
vec3 blinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    vec3 color = texture(diffuseTex, fs_in.texCoord).rgb;
    vec3 lightDirection = normalize(lightPos -  fragPos);
    vec3 viewDirection = normalize(viewPos - fragPos);
    vec3 halfwayVec = normalize(lightDirection+viewDirection);

    vec3 ambient = 0.15 * lightColor;

    float diff=  max(dot(lightDirection, normal), 0.0);

    vec3 diffuse = diff * lightColor;

    float spec = pow(max(dot(halfwayVec, normal), 0.0), 32.0);
    
    vec3 specular = spec * lightColor;

    float shadow = calculateShadow(fs_in.fragPosLightSpace);
    vec3 finalLighting = (ambient + (1.0 - shadow) * (diffuse+specular)) * color; //if we get a 1 from shadow calc, we turn off diffuse and specular components

    return finalLighting;
}


void main()
{
    FragColor = vec4(blinnPhong(fs_in.normal, fs_in.fragPos, lightPos, vec3(1.0)), 1.0);
}