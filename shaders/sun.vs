#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 sunPos;

const float SCALE = 30.f;

void main()
{
        vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
        vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
        vec3 vert_world = sunPos + cameraRight * vertex.x * SCALE + cameraUp * vertex.y * SCALE;
        TexCoords = texCoords;
        gl_Position = projection * view * vec4(vert_world, 1.0);        
        gl_Position.z = gl_Position.w - 0.001f;
}