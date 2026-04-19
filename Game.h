#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include "Material.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Light.h"
#include "Sky.h"
#include "ShadowSettings.h"

class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods
	ID3DBlob* LoadShaderBlob(const wchar_t* filePath);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(ID3DBlob* blob);
	Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(ID3DBlob* blob);
	void LoadMeshes();
	void LoadTextures();
	void LoadMaterials();
	void CreateEntities();
	void CreateCameras();
	void CreateLights();
	void CreateShadows();
	void CreateSky();

	// Drawing helper methods
	void DrawEntity(std::shared_ptr<Entity> entity, float totalTime);

	// Loaded asset data
	std::vector<std::shared_ptr<Mesh>> meshes;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneTextureM; // Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodTextureM; // Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> treadPlateTextureM; // Metalness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeTextureM; // Metalness

	// Shared input layout for shaders
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Loaded material data
	std::vector<std::shared_ptr<Material>> materials;

	// Created entity data
	std::vector<std::shared_ptr<Entity>> entities;

	// Created camera data
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCameraIndex;

	// Lighting data
	DirectX::XMFLOAT4 lightAmbient;
	std::vector<Light> lights; // All active lights in the scene

	// Shadow data
	ShadowSettings shadows;
	void DrawShadowMap();

	// Created and loaded sky data
	std::shared_ptr<Sky> sky;

	// UI helper functions
	void UpdateImGui(float deltaTime, float totalTime);
	void BuildUI();
	void BuildMeshUI(Mesh* mesh, int index);
	void BuildMaterialUI(Material* material, int index);
	void BuildEntityUI(Entity* entity, int index);
	void BuildLightUI(Light* light, int index);
};
