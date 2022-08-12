struct GS_OUTPUT
{
    float4 sv_position : SV_POSITION;
    uint rtv_id : SV_RenderTargetArrayIndex;
};

struct GS_INPUT
{
    float4 sv_position : SV_POSITION;
    uint view_id : TEXCOORD0;
};

[maxvertexcount(3)] void main(triangle GS_INPUT input[3],
                              inout TriangleStream<GS_OUTPUT> out_stream) {
    GS_OUTPUT output;
    [unroll(3)] for (int i = 0; i < 3; ++i)
    {
        output.sv_position = input[i].sv_position;
        output.rtv_id = input[i].view_id;
        out_stream.Append(output);
    }
}