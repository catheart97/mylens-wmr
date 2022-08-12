#include "VertexShaderStructures.hlsli"

struct VS_OUTPUT
{
    float4 sv_position : SV_POSITION;
    uint view_id : TEXCOORD0;
};

VS_OUTPUT main(VERTEX_DATA_ENVIRONMENT input)
{
    VS_OUTPUT output;

    int idx = input.instance_id % 2;

    output.sv_position = float4(input.position, 1.0);
    output.sv_position = mul(model_m, output.sv_position);
    output.sv_position = mul(view_m[idx], output.sv_position);
    output.sv_position = mul(proj_m[idx], output.sv_position);
    output.view_id = input.instance_id;

    return output;
}