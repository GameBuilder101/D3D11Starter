#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using VertexShader = Microsoft::WRL::ComPtr<ID3D11VertexShader>;
using PixelShader = Microsoft::WRL::ComPtr<ID3D11PixelShader>;

class Material {
public:
	Material(VertexShader vertexShader, PixelShader pixelShader, DirectX::XMFLOAT4 tint);
	~Material();
	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;

	VertexShader GetVertexShader();
	void SetVertexShader(VertexShader vertexShader);

	PixelShader GetPixelShader();
	void SetPixelShader(PixelShader pixelShader);

	DirectX::XMFLOAT4 GetTint();
	void SetTint(DirectX::XMFLOAT4 tint);

private:
	VertexShader vertexShader;
	PixelShader pixelShader;
	DirectX::XMFLOAT4 tint;
};