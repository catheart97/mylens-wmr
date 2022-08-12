#include "ShaderConstants.h"

struct VERTEX_DATA
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
    uint instance_id : SV_InstanceID;
};

struct VERTEX_DATA_ENVIRONMENT
{
    float3 position : POSITION;
    uint instance_id : SV_InstanceID;
};

cbuffer CONSTANT_VS : register(b0)
{
    matrix<float, 4, 4> model_m;
    matrix<float, 4, 4> view_m[2]; // left and right
    matrix<float, 4, 4> proj_m[2];
    matrix<float, 4, 4> normal_m;
};