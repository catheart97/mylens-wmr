#include "ShaderConstants.h"

#define PI 3.1415926538

struct PS_INPUT
{
    float4 sv_position : SV_POSITION;
    float4 position : POSITION;
    float4 normal : NORMAL;
};

cbuffer CONSTANT_PS_SOLID : register(b0)
{
    float3 albedo;
    float alpha;
}

float4 main(PS_INPUT input) : SV_TARGET { return float4(albedo, alpha); }