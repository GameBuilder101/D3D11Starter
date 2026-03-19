
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	float4 screenPosition   : SV_POSITION;
    float3 normal           : NORMAL;
    float2 uv               : TEXCOORD;
};

cbuffer ExternalData : register(b0)
{
    float2 textureScale;
    float2 textureOffset;
    float4 tint;
    float time;
}

Texture2D AlbedoMap : register(t0); // "t" registers are for textures
SamplerState MainSampler : register(s0); // "s" registers are for samplers

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Calculate scaled/offset UVs
    float2 uv = input.uv * textureScale + textureOffset;
    
    float4 albedo = AlbedoMap.Sample(MainSampler, uv);
    // Apply color tint
    albedo *= tint;
    
    return albedo;
}