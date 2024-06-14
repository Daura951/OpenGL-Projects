#version 330 core

out vec4 FragColor;

in VS_OUT
{
    vec3 fragPos;
    vec3 tangentViewPos;
    vec3 tangentLightPos;
    vec3 tangentFragPos;
    vec2 texCoord;
} fs_in;

uniform sampler2D diffuseTex;
uniform sampler2D normalMap;
uniform sampler2D displacementMap;

uniform float heightScale;


vec2 parallaxMapping(vec2 texCoord, vec3 viewDir)
{

    float minLayers = 8.0;
    float maxLayers = 32.0;
    
    //if we look head on, have less layers. If we look at an angle, add more layers
    float numOfLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0)); 
    float layerDepth = 1.0 / numOfLayers;

    float currentDepth = 0.0;

    vec2 p = viewDir.xy * heightScale; //p is a scaled viewDir
    vec2 deltaTexCoord = p / numOfLayers; //move the tex along the scaled viewDir by factors of numOfLayers

    vec2 curTexCoord = texCoord; //current texCoord
    float curDepthMapVal = texture(displacementMap, curTexCoord).r; //current displacement value

    while(currentDepth < curDepthMapVal) //steep parallax time!
    {
        curTexCoord -= deltaTexCoord; //go down scaled viewDir
        curDepthMapVal = texture(displacementMap, curTexCoord).r; //resample from map
        currentDepth+=layerDepth; //go deeper in depth
    }

    //parallax occlusion

    vec2 prevTexCoord = curTexCoord + deltaTexCoord; //get previous texCoord (self explanitory based on loop above)

    float afterDepth = curDepthMapVal - currentDepth; //get depth 1 step ahead of found depth
    float beforeDepth = texture(displacementMap, prevTexCoord).r - currentDepth + layerDepth; //resample and account for depth changes before this one

    float weight = afterDepth / (afterDepth-beforeDepth); //interpolate
    vec2 finalTexCoord = prevTexCoord * weight + curTexCoord * (1.0-weight); //linearly interpolate

    return finalTexCoord;


    // //this will effectively move the texCoord to be at the height of displacement map. We multiply by view direction to get the vector in right direction
    // float height = texture(displacementMap, texCoord).r; //get the height from disaplacement map. the red value stores stuff we need
    // return texCoord - (viewDir.xy/viewDir.z * (height*heightScale)); //apply the offset to the texture coord
}

void main()
{

    vec3 lightDirection = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos); //notice that we are in tangent space
    vec3 viewDirection = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);

    vec2 texCoord = parallaxMapping(fs_in.texCoord, viewDirection);

    if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0) //get rid of weird border artifacts due to oversampling
    discard;

    vec3 color = texture(diffuseTex, texCoord).rgb;
    

    vec3 normal = texture(normalMap, texCoord).rgb; //no need to get out of tangent space since everyone else is in tangent space. Grab straight out of normal map
    normal = normalize(normal * 2.0 - 1.0); //make NDC [-1,1]

    //normal blinn-phong stuff
    vec3 ambient = 0.1 * color;

    float diff=  max(dot(lightDirection, normal), 0.0);

    vec3 diffuse = diff * color;

    vec3 halfwayVec = normalize(lightDirection+viewDirection);
    float spec = pow(max(dot(halfwayVec, normal), 0.0), 32.0);


    vec3 specular = spec * vec3(0.2);

    FragColor = vec4(ambient+ diffuse+specular, 1.0);
}