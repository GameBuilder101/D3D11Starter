#pragma once

#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using VertexShader = Microsoft::WRL::ComPtr<ID3D11VertexShader>;
using PixelShader = Microsoft::WRL::ComPtr<ID3D11PixelShader>;
using ShaderResourceView = Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
using SamplerState = Microsoft::WRL::ComPtr<ID3D11SamplerState>;

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

	void BindShaders();

	std::unordered_map<unsigned int, ShaderResourceView> GetTextures();
	void AddTexture(unsigned int registerIndex, ShaderResourceView texture);
	std::unordered_map<unsigned int, SamplerState> GetSamplers();
	void AddSampler(unsigned int registerIndex, SamplerState sampler);

	void BindTexturesAndSamplers();

	DirectX::XMFLOAT2 GetTextureScale();
	void SetTextureScale(DirectX::XMFLOAT2 textureScale);
	DirectX::XMFLOAT2 GetTextureOffset();
	void SetTextureOffset(DirectX::XMFLOAT2 textureOffset);

	DirectX::XMFLOAT4 GetTint();
	void SetTint(DirectX::XMFLOAT4 tint);

private:
	VertexShader vertexShader;
	PixelShader pixelShader;

	// Each key corresponds to the register index that the texture/state binds to
	std::unordered_map<unsigned int, ShaderResourceView> textures;
	std::unordered_map<unsigned int, SamplerState> samplers;

	DirectX::XMFLOAT2 textureScale;
	DirectX::XMFLOAT2 textureOffset;

	DirectX::XMFLOAT4 tint;
};