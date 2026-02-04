#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// The main constant buffer definition
// --------------------------------------------------------
struct ExternalVertexData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT3 offset;
};