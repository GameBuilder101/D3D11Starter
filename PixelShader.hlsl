#include "LitSurface.hlsli"
#include "NormalMapping.hlsli"

cbuffer ExternalData : register(b0)
{
    float2 textureScale;
    float2 textureOffset;
    float4 tint;
    float3 cameraPosition;
    float time;
    float4 lightAmbient;
    
    Light lights[MAX_LIGHTS]; // All lights shining on this material
}

// "t" registers are for textures
Texture2D AlbedoMap : register(t0); // Base color
Texture2D NormalMap : register(t1); // Affects normals
Texture2D RoughnessMap : register(t2); // Affects roughness
Texture2D MetalnessMap : register(t3); // Affects metalness
Texture2D ShadowMap : register(t4);

// "s" registers are for samplers
SamplerState MainSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// Default entry point for shader compiler (input is recieved from vertex shader, output is a single color)
float4 main(VertexToPixel input) : SV_TARGET
{
    // Since inputs are linearly interpolated, they do not remain normalized. Do this to fix
    input.worldNormal = normalize(input.worldNormal);
    
    // Calculate scaled/offset UVs
    float2 transformedUVs = input.uv * textureScale + textureOffset;
    
    // Sample albedo map
    float4 albedo = AlbedoMap.Sample(MainSampler, transformedUVs);
    // Reverse the gamma correction baked into the albedo texture
    albedo.rgb = pow(albedo.rgb, 2.2f);
    // Apply color tint
    albedo *= tint;
    
    // Sample and unpack normal map
    float3 tanSpaceNormal = UnpackNormal(NormalMap.Sample(MainSampler, transformedUVs).rgb);
    // Transform to retrieve actual direction relative to surface
    float3 normal = TransformNormal(tanSpaceNormal, input.worldNormal, input.worldTangent);
    
    // Sample roughness map
    float roughness = RoughnessMap.Sample(MainSampler, transformedUVs).r;
    
    // Sample metalness map
    float metalness = MetalnessMap.Sample(MainSampler, transformedUVs).r;
    
    // Perform lighting calculations using input values
    float3 totalLight = CalcTotalLight(
        input,
        albedo,
        normal,
        roughness,
        metalness,
        cameraPosition,
        lights,
        ShadowMap,
        ShadowSampler);
    // Apply gamma correction
    totalLight.rgb = pow(totalLight.rgb, 1.0f / 2.2f);
    
    return float4(totalLight, albedo.a);
}