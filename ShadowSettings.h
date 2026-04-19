#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

// A container for the many shadow settings and API objects
struct ShadowSettings
{
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture = nullptr;
	unsigned int resolution = 2048; // Should be a power of 2

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler = nullptr;

	DirectX::XMFLOAT4X4 lightViewMatrix = DirectX::XMFLOAT4X4();
	DirectX::XMFLOAT4X4 lightProjectionMatrix = DirectX::XMFLOAT4X4();
	float projectionSize = 22.0f;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
};