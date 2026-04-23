#include "PostProcessIncludes.hlsli"

VertexToPixel main(uint id : SV_VertexID)
{
    VertexToPixel output;
    
    // Calculate the UV positions based on ID for performance
    // Equation by Chris Cascioli
    output.uv = float2((id << 1) & 2, id & 2);
    output.screenPosition = float4(output.uv, 0, 1);
    output.screenPosition.x = output.screenPosition.x * 2 - 1;
    output.screenPosition.y = output.screenPosition.y * -2 + 1;
    
    return output;
}