#pragma once

#include <memory>
#include <d3d11.h>
#include "Mesh.h"
#include "Camera.h"

class Sky
{
public:
	Sky(std::shared_ptr<Mesh> mesh,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	~Sky();
	Sky(const Sky&) = delete;
	Sky& operator=(const Sky&) = delete;

	void Draw(Camera* camera);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	void SetTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubemap);

private:
	std::shared_ptr<Mesh> mesh;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
};

// Contains only the vertex data needed for the sky
struct SkyVertexShaderConstData
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};