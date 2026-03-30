#ifndef __LIT_SURFACE__
#define __LIT_SURFACE__

#include "ShaderIncludes.hlsli"

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

// Calculates extremely primitive global illumination approximation
float3 CalcAmbientTerm(float4 albedo, float4 lightAmbient)
{
    return (albedo * lightAmbient).rgb;
}

// Calculates diffuse term given a set of lights
float3 CalcDiffuseTerm(VertexToPixel input, float4 albedo, Light light)
{
    // Calculates direction from world position to light position
    float3 dirToLight = normalize(light.position - input.worldPosition);
    
    float3 lightDir = LightDir(input, light);
    
    return saturate(dot(input.worldNormal, -lightDir)) * // Diffuse intensity, clamped to 0 - 1 range
        light.color * light.intensity * // Multiply light color
        albedo.rgb; // Pick up surface color
}

// Calculates specular term given a set of lights and camera position
float3 CalcSpecularTerm(VertexToPixel input,
    float4 albedo, float roughness,
    float3 cameraPosition, Light light)
{
    // Direction from world position to camera position
    float3 dirToCamera = normalize(cameraPosition - input.worldPosition);
    float3 lightDir = LightDir(input, light);
    
    float3 reflection = reflect(lightDir, input.worldNormal);
    // Calculate cosine between reflection and direction to viewing position
    float RdotC = saturate(dot(reflection, dirToCamera));
    
    // Apply power to increase shininess
    float shine = pow(RdotC, 256);
    
    return shine * roughness * // Multiply by roughness map value
        light.color * light.intensity * // Tint to match light color
        albedo.rgb;
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

// Calculates and adds up accumulated light from ambient, diffuse, and specular
float3 CalcTotalLight(VertexToPixel input,
    float4 albedo, float roughness,
    float3 cameraPosition,
    float4 lightAmbient, Light lights[MAX_LIGHTS])
{
    float3 ambient = CalcAmbientTerm(albedo, lightAmbient);
    float3 diffuse;
    float3 specular;
    
    // Add up cumulatively from every light source
    for (uint i = 0; i < MAX_LIGHTS; i++)
    {
        float falloff = CalculateFalloffTerm(input, lights[i]);
        
        diffuse += CalcDiffuseTerm(input, albedo, lights[i]) * falloff;
        specular += CalcSpecularTerm(input, albedo, roughness, cameraPosition, lights[i]) * falloff;
    }
    
    return ambient + diffuse + specular;
}

#endif