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

float4 main(VertexToPixel input) : SV_TARGET
{
	// Output normal vectors as colors
    return float4(input.normal.xyz, 1.0f) * tint;
}