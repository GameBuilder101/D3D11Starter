#ifndef __SKY_SHADER_INCLUDES__
#define __SKY_SHADER_INCLUDES__

// Data that gets sent from the vertex shader to the pixel shader
struct SkyVertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 sampleDirection : DIRECTION;
};

#endif