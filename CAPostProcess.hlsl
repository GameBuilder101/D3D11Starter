#include "PostProcessIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float3 channelOffsets;
    float padding;
    float2 focalPoint;
}

// "t" registers are for textures
Texture2D InputRender : register(t0);

// "s" registers are for samplers
SamplerState MainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 output;
    
    float2 dir = input.uv - focalPoint;
    
    // Sample each channel by a given offset from the focal point
    output.r = InputRender.Sample(MainSampler,
        input.uv + dir * float2(channelOffsets.r, channelOffsets.r)).r;
    output.g = InputRender.Sample(MainSampler,
        input.uv + dir * float2(channelOffsets.g, channelOffsets.g)).g;
    output.b = InputRender.Sample(MainSampler,
        input.uv + dir * float2(channelOffsets.b, channelOffsets.b)).b;
    output.a = 1.0f;
    
    return output;
}