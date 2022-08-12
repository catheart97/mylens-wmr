#include "ShaderConstants.h"
#include "LightFunctions.hlsli"

#define PI 3.1415926538

struct PS_INPUT
{
    float4 sv_position : SV_POSITION;
    float4 position : POSITION;
    float4 normal : NORMAL;
};

cbuffer CONSTANT_PS_PHONG : register(b0)
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float alpha;
    float shininess;
    float3 camera;
    float4 lights[MAX_NUMBER_LIGHTS];
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 pos = input.position.xyz;
    float3 normal = normalize(input.normal.xyz);
    float3 view_dir = normalize(camera.xyz - input.position.xyz);

    float3 color = float3(0, 0, 0);
    for (int i = 0; i < MAX_NUMBER_LIGHTS; ++i)
    {
        if (lights[i].w == 0) break; // all remainding are 0

        float3 light_pos = lights[i].xyz;
        float3 light_color = {lights[i].w, lights[i].w, lights[i].w};

        float3 attenuation = calculate_attenuation(pos, light_pos);
        float3 light_dir = light_pos - pos;

        // diffuse (ambient is given)
        float3 diff = max(dot(normal, light_dir), 0) * diffuse;

        float3 reflect_dir = reflect(-light_dir, normal);
        float3 half_way = normalize(light_dir + view_dir);
        float spec = pow(max(dot(normal, half_way), 0), shininess);

        color += ambient + attenuation * (diff + spec * specular);
    }

    return float4(color, alpha);
}