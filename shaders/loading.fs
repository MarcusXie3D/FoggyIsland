#version 330 core
out vec4 color;

in vec2 TexCoords;

uniform sampler2D loadingPicture;

void main()
{
    color = texture(loadingPicture, TexCoords);
    //color = vec4(1.0, 0.0, 0.0, 1.0);
}