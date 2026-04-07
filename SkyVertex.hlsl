#include "ShaderIncludes.hlsli"
#include "SkyShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

// Default entry point for shader compiler (input is recieved from vertex data, output is passed down)
SkyVertexToPixel main(VertexInput input)
{
    // Set up output struct
    SkyVertexToPixel output;
    
    // Retrieve view matrix without translation values
    matrix originView = view;
    originView._14 = 0;
    originView._24 = 0;
    originView._34 = 0;
    
    // Apply just a view-projection matrix (no world component)
    matrix vp = mul(projection, originView);
    output.screenPosition = mul(vp, float4(input.localPosition, 1.0f));
    // Ensure that after w division, the position is always flattened to a z of 1
    output.screenPosition.z = output.screenPosition.w;
    
    // Don't need to normalize (automatically handled for cubemap sampling)
    output.sampleDirection = input.localPosition;
    
    return output;
}