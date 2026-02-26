#include "Material.h"

Material::Material(VertexShader vertexShader, PixelShader pixelShader, DirectX::XMFLOAT4 tint)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
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

DirectX::XMFLOAT4 Material::GetTint()
{
	return tint;
}

void Material::SetTint(DirectX::XMFLOAT4 tint)
{
	this->tint = tint;
}
