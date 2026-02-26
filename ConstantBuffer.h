#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// The vertex constant buffer definition
// --------------------------------------------------------
struct VertexShaderConstData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

// --------------------------------------------------------
// The pixel constant buffer definition
// --------------------------------------------------------
struct PixelShaderConstData
{
	DirectX::XMFLOAT4 tint;
	float time;
};