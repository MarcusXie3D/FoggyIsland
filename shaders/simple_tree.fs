#version 330 core
out vec4 FragColor;

in vec2 texcoord;

uniform sampler2D texturez;

void main()
{
    if(texture(texturez, texcoord).a < 0.5)
        discard;
    FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}


