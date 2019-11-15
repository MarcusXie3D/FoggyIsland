#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;

uniform bool horizontal;

// Only 3 weights/offsets due to linear sampling (by specifying proper offsets we indirectly sample 2 pixels instead of 1)
const float offsets[3]  = float[]( 0.0, 1.3846153846, 3.2307692308 );  
const float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

const float WIDTH = 1280.0f;
const float HEIGHT = 720.0f;

void main()
{    
    vec4 result = texture(image, vec2(gl_FragCoord) / vec2(WIDTH, HEIGHT)) * weight[0];
    // vec4 result = vec4(0.0f);
    vec2 offset;
            
    for (int i=1; i<3; i++) 
    {            
        // if(horizontal)
        //     offset = vec2(0.0, offsets[i]);
        // else
        //     offset = vec2(offsets[i], 0.0);
        if(horizontal)
            offset = vec2(offsets[i], 0.0);
        else
            offset = vec2(0.0, offsets[i]);
        result += texture(image, (vec2(gl_FragCoord) + offset) / vec2(WIDTH, HEIGHT)) * weight[i];
        result += texture(image, (vec2(gl_FragCoord) -  offset)  / vec2(WIDTH, HEIGHT)) * weight[i];
    }
      
    color = result;
    //color = texture(image, vec2(gl_FragCoord) / vec2(WIDTH, HEIGHT));
    //color = texture(image, TexCoords);
    //color = vec4(1.0, 0.0, 1.0, 1.0);
    // color = texture(input, TexCoords);
    // color = vec4((vec2(gl_FragCoord) +vec2(offset[2], 0.0)) / WIDTH, 0.0, 1.0);
    // color = vec4(vec3(gl_FragCoord.x / WIDTH), 1.0);
}