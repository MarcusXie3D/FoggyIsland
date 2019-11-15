#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 model;

out vec2 texCoord;
out vec3 normal;
out vec3 worldFragPos;
out vec3 shadowFragPos;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform mat4 shadowMat;

void main()
{
    texCoord = aTexCoord;
    vec3 pos = aPos;
    if(aTexCoord.x < 0.3)
    {
        pos.x += sin(time * 1.0 + aPos.x) * 0.10;
        pos.z += cos(time / 2.0 + aPos.z) * 0.075;
    }
    vec4 FragPos = model * vec4(pos, 1.0);
    worldFragPos = FragPos.xyz;
    vec4 shadowFrag = shadowMat * vec4(worldFragPos, 1.0);
    shadowFragPos = shadowFrag.xyz;
    gl_Position = projection * view * FragPos;
    //gl_Position = projection * view * model * vec4(aPos, 1.0);

    mat3 normalModel = mat3(inverse(transpose(model)));
    normal = normalModel * aNormal;
}