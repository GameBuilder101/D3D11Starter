#include "SkyShaderIncludes.hlsli"

// "t" registers are for textures
TextureCube SkyMap : register(t0);

// "s" registers are for samplers
SamplerState MainSampler : register(s0);

float4 main(SkyVertexToPixel input) : SV_TARGET
{
    return SkyMap.Sample(MainSampler, input.sampleDirection);
}