#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
}

// Simplified shader to only output what is needed for shadow mapping
float4 main(VertexInput input) : SV_POSITION
{
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(input.localPosition, 1.0f));
}