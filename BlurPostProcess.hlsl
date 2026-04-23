#include "PostProcessIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    int blurRadius;
    float pixelWidth;
    float pixelHeight;
}

// "t" registers are for textures
Texture2D InputRender : register(t0);

// "s" registers are for samplers
SamplerState MainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Total blurred color
    float4 total = 0;
    int sampleCount = 0;
    
    // Loop through radius box
    for (int x = -blurRadius; x <= blurRadius; x++)
    {
        for (int y = -blurRadius; y <= blurRadius; y++)
        {
            // Calculate offset UV for this sample
            float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);
            
            // Add to total
            total += InputRender.Sample(MainSampler, uv);
            sampleCount++;
        }
    }
    
    // Return average
    return total / sampleCount;
}