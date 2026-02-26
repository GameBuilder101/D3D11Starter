#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include "Material.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"

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
	void LoadMaterials();
	void CreateConstBuffers();
	void CreateEntities();
	void CreateCameras();

	// Drawing helper methods
	void DrawEntity(std::shared_ptr<Entity> entity, float totalTime);

	// Loaded mesh data
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Shared input layout for shaders
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	// Constant buffers for shaders
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexShaderConstBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pixelShaderConstBuffer;

	// Loaded material data
	std::vector<std::shared_ptr<Material>> materials;

	// Created entity data
	std::vector<std::shared_ptr<Entity>> entities;

	// Created camera data
	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCameraIndex;

	// UI helper functions
	void UpdateImGui(float deltaTime, float totalTime);
	void BuildUI();
	void BuildMeshUI(Mesh* mesh, int index);
	void BuildEntityUI(Entity* entity, int index);

	// ImGui modified data
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	bool showDemoWindow;
};

