#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 worldFragPos;
in vec3 shadowFragPos;

#pragma include Light.glsl
#pragma include Fog.glsl
  
uniform sampler2D texturez;  
uniform vec3 viewPos;
uniform Light sun;
uniform sampler2D shadowMap;
uniform Fog fog;

void main()
{
    vec4 sampled = texture(texturez, texCoord);
    if(sampled.a < 0.5)
        discard;

    vec3 ambient = sun.ambientStrength * sun.ambientColor;

    vec3 Normal = normalize(normal);
    vec3 diffuse = sun.lightColor * abs(dot(Normal, sun.direction));//when light incidents on the backface of the leaf, we invert the negative diffuse value by abs()

    vec3 viewDir = normalize(viewPos - worldFragPos);
    vec3 reflectDir = reflect(-sun.direction, Normal);
    float spec = pow(abs(dot(viewDir, reflectDir)), 16);//when light incidents on the backface of the leaf, we invert the negative spec value by abs()
    vec3 specular = spec * sun.lightColor;

    //shadow
    vec3 shadowSpacePos = shadowFragPos;
    shadowSpacePos.z -= 0.001f;
    float shadow = 0.0;

    // for(float x = -0.0005; x <= 0.0005; x+= 0.00025)
    // {
    //     for(float y = -0.0005; y <= 0.0005; y+= 0.00025)
    //     {
    //         float shadowDepth = texture(shadowMap, shadowSpacePos.xy + vec2(x, y)).r;
    //         if(shadowDepth > shadowSpacePos.z)
    //             shadow += 1.0;
    //     }
    // }
    // shadow /= 25.0;// 25 is right

    const float blurSize = 0.00020;//0.00025;
    for(float x = -blurSize; x <= blurSize; x+= blurSize)
    {
        for(float y = -blurSize; y <= blurSize; y+= blurSize)
        {
            float shadowDepth = texture(shadowMap, shadowSpacePos.xy + vec2(x, y)).r;
            if(shadowDepth > shadowSpacePos.z)
                shadow += 1.0;
        }
    }
    shadow /= 9.0;

    if(shadowSpacePos.z > 1.0)
        shadow = 1.0;
    vec3 lighting = (diffuse + specular) * shadow;

    vec3 color = sampled.rgb * (ambient + lighting);
    color.x = pow(color.x, 1.0 / 3.2);
    color.y = pow(color.y, 1.0 / 3.2);
    color.z = pow(color.z, 1.0 / 3.2);

    float fogFactor = getFogFactor(fog, viewPos, worldFragPos);
    vec3 colorWithFog = mix(color, fog.Color, fogFactor);

    FragColor = vec4(colorWithFog, 1.0);
    //float shadowTemp = texture(shadowMap, shadowSpacePos.xy).r;
    //FragColor = vec4(shadowTemp, shadowTemp, shadowTemp, 1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
} 