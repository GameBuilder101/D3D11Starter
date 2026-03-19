#include "Material.h"

#include "Graphics.h"

Material::Material(VertexShader vertexShader, PixelShader pixelShader, DirectX::XMFLOAT4 tint)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;

	this->textureScale = DirectX::XMFLOAT2(1.0f, 1.0f);
	this->textureOffset = DirectX::XMFLOAT2(0.0f, 0.0f);

	this->tint = tint;
}

Material::~Material() {}

VertexShader Material::GetVertexShader()
{
	return vertexShader;
}

void Material::SetVertexShader(VertexShader vertexShader)
{
	this->vertexShader = vertexShader;
}

PixelShader Material::GetPixelShader()
{
	return pixelShader;
}

void Material::SetPixelShader(PixelShader pixelShader)
{
	this->pixelShader = pixelShader;
}

void Material::BindShaders()
{
	Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
	Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
}

std::unordered_map<unsigned int, ShaderResourceView> Material::GetTextures()
{
	return textures;
}

void Material::AddTexture(unsigned int registerIndex, ShaderResourceView texture)
{
	textures.insert({registerIndex, texture});
}

std::unordered_map<unsigned int, SamplerState> Material::GetSamplers()
{
	return samplers;
}

void Material::AddSampler(unsigned int registerIndex, SamplerState sampler)
{
	samplers.insert({registerIndex, sampler});
}

void Material::BindTexturesAndSamplers()
{
	// Bind textures
	for (auto& pair : textures)
	{
		Graphics::Context->PSSetShaderResources(pair.first, 1, pair.second.GetAddressOf());
	}

	// Bind samplers
	for (auto& pair : samplers)
	{
		Graphics::Context->PSSetSamplers(pair.first, 1, pair.second.GetAddressOf());
	}
}

DirectX::XMFLOAT2 Material::GetTextureScale()
{
	return textureScale;
}

void Material::SetTextureScale(DirectX::XMFLOAT2 textureScale)
{
	this->textureScale = textureScale;
}

DirectX::XMFLOAT2 Material::GetTextureOffset()
{
	return textureOffset;
}

void Material::SetTextureOffset(DirectX::XMFLOAT2 textureOffset)
{
	this->textureOffset = textureOffset;
}

DirectX::XMFLOAT4 Material::GetTint()
{
	return tint;
}

void Material::SetTint(DirectX::XMFLOAT4 tint)
{
	this->tint = tint;
}
