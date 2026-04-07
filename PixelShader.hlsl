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
Texture2D RoughnessMap : register(t2); // Affects specular

// "s" registers are for samplers
SamplerState MainSampler : register(s0);

// Default entry point for shader compiler (input is recieved from vertex shader, output is a single color)
float4 main(VertexToPixel input) : SV_TARGET
{
    // Since inputs are linearly interpolated, they do not remain normalized. Do this to fix
    input.worldNormal = normalize(input.worldNormal);
    
    // Calculate scaled/offset UVs
    float2 transformedUVs = input.uv * textureScale + textureOffset;
    
    // Sample albedo map
    float4 albedo = AlbedoMap.Sample(MainSampler, transformedUVs);
    // Apply color tint
    albedo *= tint;
    
    // Sample and unpack normal map
    float3 tanSpaceNormal = UnpackNormal(NormalMap.Sample(MainSampler, transformedUVs).rgb);
    // Transform to retrieve actual direction relative to surface
    float3 normal = TransformNormal(tanSpaceNormal, input.worldNormal, input.worldTangent);
    
    // Sample roughness map (scaled by 4 because test textures are too dull for previewing)
    float roughness = saturate(AlbedoMap.Sample(MainSampler, transformedUVs).r * 4.0f);
    
    // Before performing lighting, provide the new mapped normals instead
    input.worldNormal = normal;
    
    // Perform lighting calculations using input values
    float3 totalLight = CalcTotalLight(input,
        albedo, roughness,
        cameraPosition,
        lightAmbient, lights);
    
    return float4(totalLight, albedo.a);
}