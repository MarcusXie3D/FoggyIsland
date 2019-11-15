#version 330 core
out vec4 color;

in vec2 TexCoords;

uniform sampler2D scene;
uniform vec2 sunPos;
uniform int samples = 100;
//uniform float weightDecrement = 0.99;//0.95;
uniform float strength = 0.9;

void main()
{
    //float confront = 1.0;
    //int sampleNr = samples;
    // if(sunPos.z < 0.0 || sunPos.x < 0.0 || sunPos.x > 1.0 || sunPos.y < 0.0 || sunPos.y > 1.0){//when the sun is not in our sight, shut down god rays calculation. if not, with trees and other object together to render, the fps may become luggish. 
    //     confront = 0.0;
    //     sampleNr = 1;
    // }
    vec2 ray = TexCoords - sunPos;
    //float decay = 1.0 - clamp(length(ray), 0.0, 1.0);
    vec2 pace = ray / samples;
    //color = texture(scene, TexCoords);
    vec2 samplePoint = TexCoords;
    //float weight = 1.0;
    for(int i = 0; i < samples; i++)
    {
        color += texture(scene, samplePoint);// * weight;
        //weight *= weightDecrement;
        samplePoint -= pace;
    }
    color /= samples;
    color *= strength;//* decay
    //color.x += 0.5; 
    //color = texture(scene, TexCoords);
    // if(TexCoords.y > 0.666)
    //     color = vec4(0.0, 0.0, 1.0, 1.0);
    // else if(TexCoords.y < 0.333)
    //     color = vec4(1.0, 0.0, 0.0, 1.0);
    // else
    //     color = vec4(1.0, 1.0, 1.0, 1.0);

    // color = texture(scene, TexCoords);
    // if(color.x > 0.0)
    //     color = vec4(1.0, 0.0, 0.0, 1.0);
    // else
    //     color = texture(normalScene, TexCoords);
}