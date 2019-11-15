struct Fog{
    float Density;
    vec3 Color;
};

float getFogFactor(Fog fog, vec3 viewPos, vec3 fragPos){
    float distance = length(viewPos - fragPos);
    float factor = exp(-pow(fog.Density * distance, 2));
    factor = clamp(factor, 0.0, 1.0);
    return 1.0 - factor;
}