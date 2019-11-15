#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec2 texCoords;
out vec3 normal;
out vec3 shadowFragPos;
out vec3 worldFragPos;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 shadowMat;

uniform float waterHeight;
uniform bool isRefraction;
uniform bool isReflection;

void main()
{
    texCoords = aTexCoords;  
    normal = aNormal;

    worldFragPos = aPos;
    
    gl_Position = projection * view * vec4(aPos, 1.0);

    vec4 shadowFrag = shadowMat * vec4(aPos, 1.0);
    shadowFragPos = shadowFrag.xyz;

    if(isRefraction)
        gl_ClipDistance[0] = dot(vec4(aPos, 1.0), vec4(0.0, -1.0, 0.0, waterHeight + 4.0));
    if(isReflection)
        gl_ClipDistance[0] = dot(vec4(aPos, 1.0), vec4(0.0, 1.0, 0.0, -waterHeight + 0.6));
}