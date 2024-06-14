#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location=4) in vec3 aBiTangent;

uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 modelMat;

uniform vec3 lightPos;
uniform vec3 viewPos;

out VS_OUT
{
    vec3 fragPos;
    mat3 TBN;
    vec3 tangentViewPos;
    vec3 tangentLightPos;
    vec3 tangentFragPos;
    vec2 texCoord;
    vec3 normal;
} vs_out;

void main()
{
    vs_out.fragPos = vec3(modelMat *  vec4(aPos,1.0));
    vs_out.texCoord = aTexCoord;

    mat3 normalMat = transpose(inverse(mat3(modelMat)));
    vec3 T  = normalize(normalMat * aTangent); //get the tangent vector out of tangent space
    vec3 B = normalize(normalMat * aBiTangent); //get bitangent vector out of tangent space
    vec3 N = normalize(normalMat * aNormal); //get normal vector out of tangent spave
    vs_out.TBN = mat3(T,B,N); //just to have, the TBN matrix
    mat3 TBN = transpose(mat3(T,B,N)); //since its orthogonal we can transpose to get inverse (model -> tangent space)

    vs_out.tangentLightPos = TBN * lightPos; //convert to tangent space
    vs_out.tangentViewPos  = TBN * viewPos; //convert to tangent space
    vs_out.tangentFragPos  = TBN * vs_out.fragPos; //convert to tangent space

    vs_out.normal = aNormal;

    gl_Position = projectionMat * viewMat * modelMat* vec4(aPos, 1.0);
}