#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 texCoord;
out vec3 normal;
out vec3 worldFragPos;
out vec3 shadowFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 shadowMat;

void main()
{
    texCoord = aTexCoord;
    vec4 FragPos = model * vec4(aPos, 1.0);
    worldFragPos = FragPos.xyz;
    vec4 shadowFrag = shadowMat * vec4(worldFragPos, 1.0);
    shadowFragPos = shadowFrag.xyz;
    gl_Position = projection * view * FragPos;

    mat3 normalModel = mat3(inverse(transpose(model)));
    normal = normalModel * aNormal;
    // vec3 tangent = normalize(normalModel * aTangent);
    // vec3 bitangent = normalize(normalModel * aBitangent);

    // mat3 TBN = transpose(mat3(tangent, bitangent, normal));
    // tanLightDir = TBN * lightDir;
}