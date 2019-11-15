#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;

out vec4 projectionPos;
out vec2 texcoord;
out vec3 worldFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float scaleTex;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    worldFragPos = worldPos.xyz;
    projectionPos = projection * view * worldPos;
    gl_Position = projectionPos;

    texcoord = aTexcoord * scaleTex;
}