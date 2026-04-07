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
	void CreateSky();

	// Drawing helper methods
	void DrawEntity(std::shared_ptr<Entity> entity, float totalTime);

	// Loaded asset data
	std::vector<std::shared_ptr<Mesh>> meshes;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bricksTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bricksTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bricksTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> planksTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> planksTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> planksTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rocksTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rocksTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rocksTextureR; // Roughness
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> stonesTexture; // Albedo
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> stonesTextureN; // Normal
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> stonesTextureR; // Roughness

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
