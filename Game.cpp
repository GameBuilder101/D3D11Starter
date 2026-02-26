#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "ConstantBuffer.h"

#include <cmath>
#include <format>
#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Helper methods for loading assets and scene entities
	LoadMeshes();
	LoadMaterials();
	CreateConstBuffers();
	CreateEntities();
	CreateCameras();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	ImGui::StyleColorsDark();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Loads the geometry we're going to draw
// --------------------------------------------------------
void Game::LoadMeshes()
{
	meshes = {
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad_double_sided.obj").c_str())
	};
}


// --------------------------------------------------------
// Loads a pre-compiled shader code from file
// --------------------------------------------------------
ID3DBlob* Game::LoadShaderBlob(const wchar_t* filePath)
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	ID3DBlob* shaderBlob;
	// Read our compiled shader code files into blobs
	D3DReadFileToBlob(FixPath(filePath).c_str(), &shaderBlob);

	return shaderBlob;
}


// --------------------------------------------------------
// Loads a pre-compiled vertex shader
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11VertexShader> Game::LoadVertexShader(ID3DBlob* blob)
{
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
	// Create the actual Direct3D shader on the GPU
	Graphics::Device->CreateVertexShader(
		blob->GetBufferPointer(),	// Get a pointer to the blob's contents
		blob->GetBufferSize(),		// How big is that data?
		0,							// No classes in this shader
		shader.GetAddressOf());		// The address of the ID3D11VertexShader pointer

	return shader;
}


// --------------------------------------------------------
// Loads a pre-compiled pixel shader
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11PixelShader> Game::LoadPixelShader(ID3DBlob* blob)
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
	// Create the actual Direct3D shader on the GPU
	Graphics::Device->CreatePixelShader(
		blob->GetBufferPointer(),	// Pointer to blob's contents
		blob->GetBufferSize(),		// How big is that data?
		0,							// No classes in this shader
		shader.GetAddressOf());		// Address of the ID3D11PixelShader pointer

	return shader;
}


// --------------------------------------------------------
// Does the following:
// - Loads shaders into blobs
// - Loads the shader ComPtrs
// - Creates the input layout
// - Creates material definitions
// --------------------------------------------------------
void Game::LoadMaterials()
{
	// Load all the shaders that may be mixed and matched between materials
	ID3DBlob* vertexShaderBlob = LoadShaderBlob(L"VertexShader.cso");
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = LoadVertexShader(vertexShaderBlob);

	ID3DBlob* pixelShaderBlob = LoadShaderBlob(L"PixelShader.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader = LoadPixelShader(pixelShaderBlob);

	ID3DBlob* debugNormalsPSBlob = LoadShaderBlob(L"DebugNormalsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> debugNormalsPS = LoadPixelShader(debugNormalsPSBlob);

	ID3DBlob* debugUVsPSBlob = LoadShaderBlob(L"DebugUVsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> debugUVsPS = LoadPixelShader(debugUVsPSBlob);

	ID3DBlob* customPSBlob = LoadShaderBlob(L"CustomPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> customPS = LoadPixelShader(customPSBlob);

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[3] = {};

		// FLOAT3 Position
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElements[0].SemanticName = "POSITION";
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // Assume it's after the previous element

		// FLOAT3 Normal
		inputElements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElements[1].SemanticName = "NORMAL";
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		// FLOAT3 UV
		inputElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElements[2].SemanticName = "TEXCOORD";
		inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			3,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}

	/* Finally, create the materials. These will store the
	 * shaders that were just loaded */
	materials = {
		// Solid red
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)),
		// Solid green
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)),
		// Solid blue
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)),
		// Normals display
		std::make_shared<Material>(vertexShader, debugNormalsPS, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
		// UVs display
		std::make_shared<Material>(vertexShader, debugUVsPS, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
		// Custom
		std::make_shared<Material>(vertexShader, customPS, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
	};
}


// --------------------------------------------------------
// Creates the constant buffer for shaders to
// access constant data updated per object
// --------------------------------------------------------
void Game::CreateConstBuffers()
{
	unsigned int size;
	D3D11_BUFFER_DESC cbd;

	// Create the vertex shader buffer
	{
		// Calculate the size of the vertex constant buffer (snapped to multiple of 16)
		size = (sizeof(VertexShaderConstData) + 15) / 16 * 16;

		// Describe the constant buffer
		cbd = {}; // Sets struct to all zeros
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.ByteWidth = size; // Must be a multiple of 16
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.Usage = D3D11_USAGE_DYNAMIC;

		Graphics::Device->CreateBuffer(&cbd, 0, vertexShaderConstBuffer.GetAddressOf());
		// Bind the buffer to vertex register 0 for now
		Graphics::Context->VSSetConstantBuffers(0, 1, vertexShaderConstBuffer.GetAddressOf());
	}

	// Create the pixel shader buffer
	{
		// Calculate the size of the pixel constant buffer (snapped to multiple of 16)
		size = (sizeof(PixelShaderConstData) + 15) / 16 * 16;

		// Describe the constant buffer
		cbd = {}; // Sets struct to all zeros
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.ByteWidth = size; // Must be a multiple of 16
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.Usage = D3D11_USAGE_DYNAMIC;

		Graphics::Device->CreateBuffer(&cbd, 0, pixelShaderConstBuffer.GetAddressOf());
		// Bind the buffer to pixel register 0 for now
		Graphics::Context->PSSetConstantBuffers(0, 1, pixelShaderConstBuffer.GetAddressOf());
	}
}


// --------------------------------------------------------
// Creates entities to be rendered
// --------------------------------------------------------
void Game::CreateEntities()
{
	unsigned int gridWidth = (unsigned int)meshes.size();
	unsigned int gridHeight = 3;
	float gridSpacing = 3.0f;
	float gridXOffset = -(float)(gridWidth - 1) * gridSpacing * 0.5f;
	float gridYOffset = -(float)(gridHeight - 1) * gridSpacing * 0.5f;

	/* Since the entity list is being auto-generated, we need some
	 * way to define which one uses which material */
	unsigned int materialIndices[] = {
		3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4,
		0, 1, 2, 5, 0, 1, 2
	};

	unsigned int i = 0; // Track the index of the entity being made
	// Create entities in a grid for display purposes
	for (int y = gridHeight - 1; y >= 0; y--)
	{
		for (unsigned int x = 0; x < gridWidth; x++)
		{
			// Make the entity
			entities.push_back(std::make_shared<Entity>(
				meshes[x],
				materials[materialIndices[i]]));

			// Position the entity in the grid
			entities[i]->GetTransform()->SetPosition(
				x * gridSpacing + gridXOffset,
				y * gridSpacing + gridYOffset,
				0.0f);
			// Rotate the direction of each entity
			entities[i]->GetTransform()->SetRotation(0.0f, XM_PI * 0.75f, 0.0f);

			i++;
		}
	}
}


// --------------------------------------------------------
// Creates cameras to swap between
// --------------------------------------------------------
void Game::CreateCameras()
{
	float aspectRatio = Window::AspectRatio();

	// Create different cameras for testing
	cameras = {
		std::make_shared<Camera>(aspectRatio, XMFLOAT3(0.0f, 0.0f, -8.0f), XMFLOAT3(), XM_PIDIV4),
		std::make_shared<Camera>(aspectRatio, XMFLOAT3(0.0f, -8.0f, -8.0f), XMFLOAT3(-XM_PIDIV4, 0.0f, 0.0f), XM_PI * 0.15f),
		std::make_shared<Camera>(aspectRatio, XMFLOAT3(0.0f, 8.0f, -8.0f), XMFLOAT3(XM_PIDIV4, 0.0f, 0.0f), XM_PI * 0.35f)
	};
	activeCameraIndex = 0;
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Update all cameras
	for (unsigned int i = 0; i < cameras.size(); i++)
	{
		cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime, totalTime);
	BuildUI();

	// Update active camera
	cameras[activeCameraIndex]->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	{
		for (unsigned int i = 0; i < entities.size(); i++)
		{
			DrawEntity(entities[i], totalTime);
		}
	}

	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}


// --------------------------------------------------------
// Helper function called for each entity during Draw()
// --------------------------------------------------------
void Game::DrawEntity(std::shared_ptr<Entity> entity, float totalTime)
{
	// Bind the current shaders based on the entity's materials
	Graphics::Context->VSSetShader(entity->GetMaterial()->GetVertexShader().Get(), 0, 0);
	Graphics::Context->PSSetShader(entity->GetMaterial()->GetPixelShader().Get(), 0, 0);

	// Update vertex shader constant buffer values
	{
		// Set up the buffer data struct
		VertexShaderConstData externalData = {};
		externalData.world = entity->GetTransform()->GetWorldMatrix();
		externalData.view = cameras[activeCameraIndex]->GetViewMatrix();
		externalData.projection = cameras[activeCameraIndex]->GetProjectionMatrix();

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		// Lock memory
		Graphics::Context->Map(vertexShaderConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		// Copy data into constant memory on the GPU
		memcpy(mappedBuffer.pData, &externalData, sizeof(externalData));
		// Unlock memory
		Graphics::Context->Unmap(vertexShaderConstBuffer.Get(), 0);
	}

	// Update pixel shader constant buffer values
	{
		// Set up the buffer data struct
		PixelShaderConstData externalData = {};
		externalData.tint = entity->GetMaterial()->GetTint();
		externalData.time = totalTime;

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		// Lock memory
		Graphics::Context->Map(pixelShaderConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		// Copy data into constant memory on the GPU
		memcpy(mappedBuffer.pData, &externalData, sizeof(externalData));
		// Unlock memory
		Graphics::Context->Unmap(pixelShaderConstBuffer.Get(), 0);
	}

	// Perform the draw call on the entity's mesh
	entity->GetMesh()->Draw();
}


// --------------------------------------------------------
// Helper function for updating ImGui
// --------------------------------------------------------
void Game::UpdateImGui(float deltaTime, float totalTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}


// --------------------------------------------------------
// Helper function for building ImGui widgets
// --------------------------------------------------------
void Game::BuildUI()
{
	ImGui::Begin("Inspector"); // Everything after is part of the window

	// Display current frame rate (%f gets replaced)
	ImGui::Text("Frame rate: %f fps", ImGui::GetIO().Framerate);

	// Display the current window size (%d replaced in order)
	ImGui::Text("Window client size: %dx%d", Window::Width(), Window::Height());

	// Color picker for current window background
	ImGui::ColorEdit4("Background color", backgroundColor);

	// Toggles the demo window
	if (ImGui::Button("Toggle ImGui demo window"))
		showDemoWindow = !showDemoWindow;

	// Show camera data and swapping
	if (ImGui::TreeNode("Cameras"))
	{
		std::shared_ptr<Camera> active = cameras[activeCameraIndex];
		XMFLOAT3 position = active->GetTransform()->GetPosition();
		XMFLOAT3 rotation = active->GetTransform()->GetRotation();

		ImGui::Text("Position: %f, %f, %f", position.x, position.y, position.z);
		ImGui::Text("Rotation: %f, %f, %f", rotation.x, rotation.y, rotation.z);
		ImGui::Text("Aspect Ratio: %f", active->GetAspectRatio());
		ImGui::Text("FOV: %f", active->GetFOV());

		ImGui::Text("Swap Active Camera:");
		// Create inputs for swapping the current camera
		for (unsigned int i = 0; i < cameras.size(); i++)
		{
			ImGui::RadioButton(std::format("Camera {}", i).c_str(), &activeCameraIndex, i);
		}

		ImGui::TreePop();
	}

	// Show a panel for displaying debug information on the meshes
	if (ImGui::TreeNode("Meshes"))
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			BuildMeshUI(meshes[i].get(), i);
		}

		ImGui::TreePop();
	}

	// Show a panel for modifying entity data
	if (ImGui::TreeNode("Entities"))
	{
		for (unsigned int i = 0; i < entities.size(); i++)
		{
			BuildEntityUI(entities[i].get(), i);
		}

		ImGui::TreePop();
	}

	ImGui::End(); // Ends the current window

	// Show the demo window
	if (showDemoWindow)
		ImGui::ShowDemoWindow();
}


// Build a UI to display debug information about a mesh
void Game::BuildMeshUI(Mesh* mesh, int index)
{
	if (!ImGui::TreeNode(std::format("Mesh {}", index).c_str()))
		return;

	ImGui::Text("Triangles: %d", mesh->GetIndexBufferCount() / 3);
	ImGui::Text("Vertices: %d", mesh->GetVertexBufferCount());
	ImGui::Text("Indices: %d", mesh->GetIndexBufferCount());

	ImGui::TreePop();
}


// Build a UI to control transform data of entities
void Game::BuildEntityUI(Entity* entity, int index)
{
	if (!ImGui::TreeNode(std::format("Entity {}", index).c_str()))
		return;

	Transform* transform = entity->GetTransform();

	// Get the position in a local value
	DirectX::XMFLOAT3 v = transform->GetPosition();
	// DragFloat3 might modift the value
	ImGui::DragFloat3("Position", &v.x, 0.1f);
	// Assign the value back to the transform
	transform->SetPosition(v);

	v = transform->GetRotation();
	ImGui::DragFloat3("Rotation (Radians)", &v.x, 0.1f);
	transform->SetRotation(v);

	v = transform->GetScale();
	ImGui::DragFloat3("Scale", &v.x, 0.1f);
	transform->SetScale(v);

	// Add label for debug information
	ImGui::Text("Mesh Index Count: %d", entity->GetMesh()->GetIndexBufferCount());

	ImGui::TreePop();
}
