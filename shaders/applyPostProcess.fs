#version 330 core
out vec4 color;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D normalScene;

void main()
{
    color = texture(normalScene, TexCoords) + texture(scene, TexCoords);
    //color = texture(scene, TexCoords);
}