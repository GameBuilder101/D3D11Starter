#ifndef __SHADER_INCLUDES__
#define __SHADER_INCLUDES__

// Struct representing a single vertex worth of data
struct VertexInput
{
    float3 localPosition : POSITION; // XYZ position
    float3 normal : NORMAL; // Normal vector
    float2 uv : TEXCOORD; // UV texture coordinates
};

// Data that gets sent from the vertex shader to the pixel shader
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 worldPosition : POSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD;
};

#endif