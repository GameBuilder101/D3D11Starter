#ifndef __POST_PROCESS_INCLUDES__
#define __POST_PROCESS_INCLUDES__

// Data that gets sent from the vertex shader to the pixel shader
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
};

#endif