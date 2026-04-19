#ifndef __LIT_SURFACE__
#define __LIT_SURFACE__

#include "ShaderIncludes.hlsli"
#include "MathConstants.hlsli"

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_LIGHTS 5

// Contains light source data to be sent to pixel shader
struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotInnerAngle;
    float spotOuterAngle;
    float2 padding;
};

// Depending on the light type, retrieves the light direction
float3 LightDir(VertexToPixel input, Light light)
{
    switch (light.type)
    {
        default:
            return normalize(light.direction);
        case LIGHT_TYPE_POINT:
            return normalize(input.worldPosition - light.position);
    }
}

// Calculates diffuse term given a set of lights
float3 CalcDiffuseTerm(VertexToPixel input, float3 normal, Light light)
{
    // Calculate direction of the light itself
    float3 lightDir = LightDir(input, light);
    
    // Diffuse intensity, clamped to 0 - 1 range
    return saturate(dot(normal, -lightDir));
}

float3 CalcSpecularColor(float4 albedo, float metalness)
{
    // Specular color is constant for non-metals, but tinted for metals
    return lerp(0.04f, albedo.rgb, metalness);
}

// GGX (Trowbridge-Reitz)
float NormalDistribution(float3 normal, float3 halfAngle, float roughness)
{
    float nDotH = saturate(dot(normal, halfAngle));
    float nDotH2 = nDotH * nDotH;
    float a = roughness * roughness; // Remap roughness for perceptual linearity
    float a2 = max(a * a, 0.0000001f); // Clamp min to avoid 0 division
    float denominator = nDotH2 * (a2 - 1.0f) + 1.0f;
    
    // Final distribution
    return a2 / (PI * denominator * denominator);
}

// Shlick approximation
float3 Fresenel(float3 viewDir, float3 halfAngle, float3 specColor)
{
    float vDotH = saturate(dot(viewDir, halfAngle));
    
    // Final fresnel approximation
    return specColor + (1.0f - specColor) * pow(1.0f - vDotH, 5.0f);
}

// Shlick-GGX
float GeometricShadowing(float3 normal, float3 viewDir, float roughness)
{
    // End result of roughness remapping
    float k = pow(roughness + 1.0f, 2.0f) / 8.0f;
    float nDotV = saturate(dot(normal, viewDir));
    
    // Final geometric shadowing
    return 1.0f / (nDotV * (1.0f - k) + k);
}

// Combines the three components of the Cook-Terrance BRDF
float3 MicrofacetBRDF(float3 normal, float3 lightDir, float3 viewDir, float roughness, float3 specColor)
{
    // Angle between view and light
    float3 halfAngle = normalize(viewDir + lightDir);
    
    float d = NormalDistribution(normal, halfAngle, roughness);
    float3 f = Fresenel(viewDir, halfAngle, specColor);
    float g = GeometricShadowing(normal, viewDir, roughness) *
        GeometricShadowing(normal, lightDir, roughness);
    
    float3 specularResult = (d * f * g) / 4.0f;
    return specularResult * saturate(dot(normal, lightDir));
}

// Calculates specular term given a set of lights and camera position
float3 CalcSpecularTerm(
    VertexToPixel input,
    float4 albedo,
    float3 normal,
    float roughness,
    float metalness,
    float3 cameraPosition,
    Light light)
{
    // Direction from world position to camera position
    float3 viewDir = normalize(cameraPosition - input.worldPosition);
    // Calculate direction of the light itself
    float3 lightDir = -LightDir(input, light);
    
    float3 specColor = CalcSpecularColor(albedo, metalness);
    
    return MicrofacetBRDF(normal, lightDir, viewDir, roughness, specColor);
}

// Attenuates a point light, reaching zero at max range
float AttenuatePoint(VertexToPixel input, Light light)
{
    float dist = distance(light.position, input.worldPosition);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

// Attenuates a spot light, reaching zero at max angle
float AttenuateSpot(VertexToPixel input, Light light)
{
    float3 lightToPixel = normalize(input.worldPosition - light.position);
    float3 lightDir = LightDir(input, light);
    
    // Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(lightToPixel, lightDir));
    
    // Get cosines of angles and calculate range
    float cosOuter = cos(light.spotOuterAngle);
    float cosInner = cos(light.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    
    // Linear falloff over the range, clamp 0 - 1, apply to light calculation
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    return AttenuatePoint(input, light) * spotTerm;
}

// Depending on the light type, returns a different overall falloff term
float CalculateFalloffTerm(VertexToPixel input, Light light)
{
    switch (light.type)
    {
        default:
            return 1.0f;
        case LIGHT_TYPE_POINT:
            return AttenuatePoint(input, light);
        case LIGHT_TYPE_SPOT:
            return AttenuateSpot(input, light);
    }
}

// Returns 0 when in shadow map, 1 while not
float GetShadowMapTerm(VertexToPixel input, Texture2D shadowMap, SamplerComparisonState samplerState)
{
    // Perform the perspective divide (divide by W) manually
    input.shadowPosition /= input.shadowPosition.w;
    
    // Convert normalized device coordinates to usable UVs
    float2 shadowUV = input.shadowPosition.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    
    // Get distance from the light to the pixel, as well as the light to the nearest surface
    float distToLight = input.shadowPosition.z;
    return shadowMap.SampleCmpLevelZero(samplerState, shadowUV, distToLight).r;
}

// Fix diffuse lighting to account for energy conservation
float3 ConserveDiffuseEnergy(
    float3 diffuse,
    float3 fresnel,
    float metalness)
{
    return diffuse * (1.0f - fresnel) * (1.0f - metalness);
}

// Calculates and adds up accumulated light from diffuse and specular
float3 CalcTotalLight(
    VertexToPixel input,
    float4 albedo,
    float3 normal,
    float roughness,
    float metalness,
    float3 cameraPosition,
    Light lights[MAX_LIGHTS],
    Texture2D shadowMap,
    SamplerComparisonState shadowSampler)
{
    float3 totalLight;
    
    // Add up cumulatively from every light source
    for (uint i = 0; i < MAX_LIGHTS; i++)
    {
        // Direction from world position to camera position
        float3 viewDir = normalize(cameraPosition - input.worldPosition);
        // Calculate direction of the light itself
        float3 lightDir = -LightDir(input, lights[i]);
        // Angle between view and light
        float3 halfAngle = normalize(viewDir + lightDir);
        
        float3 specColor = CalcSpecularColor(albedo, metalness);
        // Fresnel needed for energy conservation in addition to specular
        float3 fresnel = Fresenel(viewDir, halfAngle, specColor);
        
        float3 diffuse = CalcDiffuseTerm(input, normal, lights[i]);
        float3 specular = CalcSpecularTerm(
            input,
            albedo,
            normal,
            roughness,
            metalness,
            cameraPosition,
            lights[i]);
        
        // Adjust diffuse for physical accuracy
        diffuse = ConserveDiffuseEnergy(diffuse, fresnel, metalness);
        
        // Light falloff
        float falloff = CalculateFalloffTerm(input, lights[i]);
        
        float shadowMapTerm = 1.0f;
        if (i == 0)
        {
            shadowMapTerm = GetShadowMapTerm(input, shadowMap, shadowSampler);
        }
        
        // Combine the diffuse and specular with light properties
        totalLight += (diffuse * albedo.rgb + specular) *
            lights[i].color * lights[i].intensity * falloff * shadowMapTerm;

    }
    
    return totalLight;
}

#endif