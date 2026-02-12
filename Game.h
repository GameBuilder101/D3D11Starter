#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include "Entity.h"

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

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void CreateEntities();

	// UI helper functions
	void UpdateImGui(float deltaTime, float totalTime);
	void BuildUI();
	void BuildMeshUI(Mesh* mesh, const char name[]);
	void BuildEntityUI(Entity* entity, int index);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Constant buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Mesh data
	std::shared_ptr<Mesh> triMesh;
	std::shared_ptr<Mesh> quadMesh;
	std::shared_ptr<Mesh> pentagonMesh;

	std::vector<std::shared_ptr<Entity>> entities;

	// ImGui modified data
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	bool showDemoWindow;
	float vertexColorTint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

