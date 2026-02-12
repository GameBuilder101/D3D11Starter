#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// The main constant buffer definition
// --------------------------------------------------------
struct ExternalVertexData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4 colorTint;
};