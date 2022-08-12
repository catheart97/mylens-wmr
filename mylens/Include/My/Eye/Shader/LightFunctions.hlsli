float calculate_attenuation(float3 p, float3 l)
{
    float distance = length(l - p);
    return 1.0 / (distance * distance); // energy preserving
}