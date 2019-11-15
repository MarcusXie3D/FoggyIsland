#version 330 core
out vec4 FragColor;

in vec3 texCoords;

#pragma include Fog.glsl
  
uniform samplerCube skyboxDiurnal; 
uniform samplerCube skyboxNocturnal; 

uniform float coeDiurnal;
uniform Fog fog;

void main()
{
    vec3 color = mix(texture(skyboxNocturnal, texCoords), texture(skyboxDiurnal, texCoords), coeDiurnal).rgb;
    color.x = pow(color.x, 1.0 / 2.2);
    color.y = pow(color.y, 1.0 / 2.2);
    color.z = pow(color.z, 1.0 / 2.2);

    float start = 0.3f; 
    float end = 0.0f;
    float distance = texCoords.y >= 0.0f ? texCoords.y : 0.0f; // TexCoords are equal to FragPos
    float fogFactor = (end - distance) / (end - start);
    fogFactor = 1.0f - clamp(fogFactor, 0.0f, 1.0f);

    vec3 colorWithFog = mix(color, fog.Color, fogFactor);
    FragColor = vec4(colorWithFog, 1.0);
    //FragColor = vec4(color, 1.0);
    //FragColor = texture(skyboxDiurnal, vec3(1.0, 0.0, 0.0));
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
} 