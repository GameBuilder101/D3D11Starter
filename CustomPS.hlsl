struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer ExternalData : register(b0)
{
    float4 tint;
    float time;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Create slow side-sweeping radar
    color.r = 1.0f - abs((1.0f - input.uv.x) * 2.0f + time * 0.5f) % 1.0f;
    // Create fast down-sweeping radar
    color.g = 1.0f - abs((1.0f - input.uv.y) * 4.0f + time * 2.0f) % 1.0f;
    // Create fast radar barber pole effect
    color.b = abs((1.0f - input.uv.x) * 0.5f + (1.0f - input.uv.y) - time) % 0.25f * 4.0f;
    
    return color * tint;
}