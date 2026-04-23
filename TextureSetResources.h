#pragma once

#include <d3d11.h>
#include <wrl/client.h>

// A container for material texture API objects
struct TextureSetResources
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone = nullptr; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneN = nullptr; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneR = nullptr; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneM = nullptr; // Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wood = nullptr; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodN = nullptr; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodR = nullptr; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodM = nullptr; // Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlate = nullptr; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateN = nullptr; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateR = nullptr; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateM = nullptr; // Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze = nullptr; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeN = nullptr; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeR = nullptr; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeM = nullptr; // Metalness
};