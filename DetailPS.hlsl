struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer ExternalData : register(b0)
{
    float2 textureScale;
    float2 textureOffset;
    float4 tint;
    float time;
}

Texture2D AlbedoMap : register(t0);
Texture2D DetailMap : register(t1);
SamplerState MainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Calculate scaled/offset UVs
    float2 uv = input.uv * textureScale + textureOffset;
    
    float4 albedo = AlbedoMap.Sample(MainSampler, uv);
    // Apply color tint
    albedo *= tint;
    
    float4 detail = DetailMap.Sample(MainSampler, uv);
    
    // Apply the detail map overtop the albedo using the alpha layer as a mask
    return lerp(albedo, detail, detail.a);
}