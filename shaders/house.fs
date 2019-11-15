#version 330 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float          shininess;
};

out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 worldFragPos;
in vec3 shadowFragPos;

#pragma include Light.glsl
#pragma include Fog.glsl
  
uniform Material material;  
uniform sampler2D lightDepthTexture;
uniform sampler2D shadowMap;
uniform vec3 viewPos;
uniform Light sun;
uniform Fog fog;

void main()
{
    vec3 ambient = sun.ambientStrength * sun.ambientColor;

    // vec3 Normal = texture(material.texture_normal1, texCoord).rgb;
    // Normal = normalize(Normal * 2.0 - 1.0);
    // vec3 LightDir = normalize(tanLightDir);
    vec3 Normal = normalize(normal);
    vec3 diffuse = sun.lightColor * max(dot(Normal, sun.direction), 0.0);

    //shadow
    vec3 shadowSpacePos = shadowFragPos;
    shadowSpacePos.z -= 0.001f;
    float shadow = 0.0;
    for(float x = -0.0005; x <= 0.0005; x+= 0.00025)
    {
        for(float y = -0.0005; y <= 0.0005; y+= 0.00025)
        {
            float shadowDepth = texture(shadowMap, shadowSpacePos.xy + vec2(x, y)).r;
            if(shadowDepth > shadowSpacePos.z)
                shadow += 1.0;
        }
    }
    shadow /= 25.0;// 25 is right
    if(shadowSpacePos.z > 1.0)
        shadow = 1.0;
    diffuse *= shadow;

    vec3 color = texture(material.texture_diffuse1, texCoord).rgb * (ambient + diffuse);
    color.x = pow(color.x, 1.0 / 2.2);
    color.y = pow(color.y, 1.0 / 2.2);
    color.z = pow(color.z, 1.0 / 2.2);

    float fogFactor = getFogFactor(fog, viewPos, worldFragPos);
    vec3 colorWithFog = mix(color, fog.Color, fogFactor);

    FragColor = vec4(colorWithFog, 1.0);
    // vec3 test = (Normal + 1.0) * 0.5;
    // FragColor = vec4(test, 1.0);
    //FragColor = vec4(0.7, 0.1, 0.4, 1.0);
} 