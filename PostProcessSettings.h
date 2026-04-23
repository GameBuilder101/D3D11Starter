#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

// A container for the API objects used for post-processing
struct PostProcessSettings
{
	// Post-process buffer is the one the scene is initially rendered to
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bufferSRV = nullptr;

	/* A second buffer is used in order to composite multiple
	 * effects. Would be swapped back-and-forth */
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> secondBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> secondBufferSRV = nullptr;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler = nullptr;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;

	// Blur
	Microsoft::WRL::ComPtr<ID3D11PixelShader> blurPixelShader = nullptr;
	int blurRadius = 0;

	// Chromatic aberration
	Microsoft::WRL::ComPtr<ID3D11PixelShader> caPixelShader = nullptr;
	DirectX::XMFLOAT3 caChannelOffsets = DirectX::XMFLOAT3(0.009f, 0.006f, -0.006f);
	DirectX::XMFLOAT2 caFocalPoint = DirectX::XMFLOAT2(0.5f, 0.5f);
};
