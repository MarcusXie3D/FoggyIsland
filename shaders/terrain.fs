#version 330 core
out vec4 FragColor;

in vec2 texCoords;
in vec3 normal;
in vec3 shadowFragPos;
in vec3 worldFragPos;

#pragma include Light.glsl
#pragma include Fog.glsl
  
uniform sampler2D terrain; 
uniform sampler2D shadowMap;
uniform vec3 viewPos;
uniform Light sun;
uniform Fog fog;
uniform bool isRefraction;

void main()
{
    vec3 ambient = sun.ambientStrength * sun.ambientColor;

    vec3 Normal = normalize(normal);
    float dotLight = dot(Normal, sun.direction);
    vec3 diffuse = sun.lightColor * max(dotLight, 0.0);

    //shadow
    if((dotLight > 0.0) && (!isRefraction))//skip shadow calculation if a fragment is not facing the sun, or under water surface
    {
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
        // shadow /= 25.0;

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
        diffuse *= shadow;
    }

    vec3 color = texture(terrain, texCoords).rgb * (ambient + diffuse);
    
    color.x = pow(color.x, 1.0 / 2.2);
    color.y = pow(color.y, 1.0 / 2.2);
    color.z = pow(color.z, 1.0 / 2.2);

    float fogFactor = getFogFactor(fog, viewPos, worldFragPos);
    vec3 colorWithFog = mix(color, fog.Color, fogFactor);
    FragColor = vec4(colorWithFog, 1.0);
    


    //FragColor = vec4(sun.lightColor, 1.0);
    // if(facingSun)
    //     FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // else
    //     FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    //FragColor = vec4(fogFactor, fogFactor, fogFactor, 1.0);

    // if(shadow != 1.0)
    //     FragColor = vec4(0.2, 0.2, 0.2, 1.0);
    // else
    //     FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    // FragColor = vec4(shadow, shadow, shadow, 1.0);
} 