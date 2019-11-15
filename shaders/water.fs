#version 330 core
out vec4 FragColor;

in vec4 projectionPos;
in vec2 texcoord;
in vec3 worldFragPos;

#pragma include Light.glsl
#pragma include Fog.glsl

uniform sampler2D refraction;
uniform sampler2D dudvMap;
uniform sampler2D reflection;
uniform sampler2D normalMap;
uniform float time;
uniform vec3 viewPos;
// uniform vec3 lightDir;
// uniform vec3 lightSpecularColor;
uniform Light sun;
uniform Fog fog;

const float DISTORTION_SCALE = 0.01;
const float WAVE_SCALE = 0.03;
//const vec3 OCEAN_BLUE = vec3(0.0078, 0.2157, 1.0);

void main()
{
    float invW = 1.0 / projectionPos.w;
    vec2 texCoordRefX = vec2((projectionPos.x * invW + 1.0) * 0.5, (projectionPos.y * invW + 1.0) * 0.5);
    vec4 distortion = texture(dudvMap, texcoord) * DISTORTION_SCALE;
    distortion.x = pow(distortion.x, 0.781);
    distortion.y = pow(distortion.y, 0.781);

    // refraction & reflection
    vec4 dudvColor = texture(dudvMap, texcoord + time + distortion.xy);
    dudvColor.x = pow(dudvColor.x, 0.781);
    dudvColor.y = pow(dudvColor.y, 0.781);
    dudvColor = dudvColor * 2.0 - 1.0;
    dudvColor = normalize(dudvColor) * WAVE_SCALE;
    texCoordRefX = clamp(texCoordRefX + dudvColor.xy, 0.001, 0.999);
    vec3 refractColor = texture(refraction, texCoordRefX).rgb;
    vec3 reflectColor = texture(reflection, texCoordRefX).rgb;

    // highlight
    vec3 normal = texture(normalMap, texcoord + time + distortion.xy).xyz;
    normal = normalize(normal * 2.0 - 1.0);
    normal = vec3(normal.x, normal.z, -normal.y);// transfer normal from tangent space into world space
    vec3 viewDir = normalize(viewPos - worldFragPos);
    vec3 halfDir = normalize(sun.direction + viewDir);
    float spec = pow(clamp(dot(halfDir, normal), 0.0, 1.0), 128);
    float fresnel = viewDir.y;// i.e. viewDir.y = dot(vec3(0.0, 1.0, 0.0), viewDir)
    vec3 specular = spec * sun.lightColor;

    vec3 color = mix((reflectColor + specular), refractColor, fresnel);

    float fogFactor = getFogFactor(fog, viewPos, worldFragPos);
    vec3 colorWithFog = mix(color, fog.Color, fogFactor);

    FragColor = vec4(colorWithFog, 1.0);
    //FragColor = vec4(reflectColor, 1.0);
    //FragColor = vec4(refractColor, 1.0);
    //FragColor = vec4(specular, 1.0);
} 