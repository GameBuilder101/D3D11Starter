#pragma once

#include <DirectXMath.h>
#include "Light.h"

// --------------------------------------------------------
// The vertex constant buffer definition
// --------------------------------------------------------
struct VertexShaderConstData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInvTranspose;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	DirectX::XMFLOAT4X4 lightView;
	DirectX::XMFLOAT4X4 lightProjection;
};

// --------------------------------------------------------
// The pixel constant buffer definition
// --------------------------------------------------------
struct PixelShaderConstData
{
	DirectX::XMFLOAT2 textureScale;
	DirectX::XMFLOAT2 textureOffset;
	DirectX::XMFLOAT4 tint;
	DirectX::XMFLOAT3 cameraPosition;
	float time;
	DirectX::XMFLOAT4 lightAmbient;

	Light lights[5];
};