#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 aTexcoord;
layout(location = 3) in mat4 model;

out vec2 texcoord;

uniform mat4 lightMatrix;
uniform float time;

void main()
{
    vec3 pos = vertex;
    texcoord = aTexcoord;
    if(aTexcoord.x < 0.3)
    {
        pos.x += sin(time * 1.0 + vertex.x) * 0.10;
        pos.z += cos(time / 2.0 + vertex.z) * 0.075;
    }
	gl_Position = lightMatrix * model * vec4(pos, 1.0);
}