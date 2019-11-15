#version 330 core
layout (location = 0) in vec3 vertex; // <vec2 position, vec2 texCoords>
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(vertex * 2.0f, 1.0f);  // scale by 2.0 since plane is specified as -0.5 - 0.5 => should be -1.0 - 1.01
    TexCoords = texCoords;
}  