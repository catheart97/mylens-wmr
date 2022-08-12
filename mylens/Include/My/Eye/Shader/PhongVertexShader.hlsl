#include "VertexShaderStructures.hlsli"

struct VS_OUTPUT
{
    float4 sv_position : SV_POSITION;
    float4 position : POSITION;
    float4 normal : NORMAL;

    uint view_id : TEXCOORD0;
};

VS_OUTPUT main(VERTEX_DATA input)
{
    VS_OUTPUT output;

    int idx = input.instance_id % 2;

    output.position = float4(input.position, 1.0);
    output.position = mul(model_m, output.position);
    output.sv_position = mul(view_m[idx], output.position);
    output.sv_position = mul(proj_m[idx], output.sv_position);
    output.normal = mul(normal_m, float4(input.normal, 1.0));

    output.view_id = input.instance_id;

    return output;
}