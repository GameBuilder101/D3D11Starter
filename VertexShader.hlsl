#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix worldInvTranspose;
    matrix view;
    matrix projection;
}

// Default entry point for shader compiler (input is recieved from vertex data, output is passed down)
VertexToPixel main(VertexInput input)
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component
    matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	
	// World position is obtained by using only the world part of the wvp matrix
    output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;
	
	/* Input normals need to be transformed by the world inverse transpose matrix,
	 * otherwise, translation or non-uniform scaling would break the normals */
    output.worldNormal = mul((float3x3)worldInvTranspose, input.normal);
	
    output.uv = input.uv;
	
    output.worldTangent = mul((float3x3)world, input.tangent);

	// Whatever we return will progress to the next stage we're using (the pixel shader for now)
	return output;
}