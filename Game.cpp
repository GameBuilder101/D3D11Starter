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
#include <WICTextureLoader.h>

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
	LoadTextures();
	LoadMaterials();
	CreateEntities();
	CreateCameras();
	CreateLights();
	CreateSky();

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
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Cube.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Cylinder.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Helix.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Sphere.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Torus.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/Quad.obj").c_str()),
		std::make_shared<Mesh>(FixPath("../../Assets/Meshes/QuadDoubleSided.obj").c_str())
	};
}


// --------------------------------------------------------
// Loads the textures shared by materials
// --------------------------------------------------------
void Game::LoadTextures()
{
	// Bricks texture
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(), // Context needed for mipmap creation
		L"Assets/Textures/Bricks.png", // File path is automatically relative
		0, // ID3D11Texture2D pointer, not used
		bricksTexture.GetAddressOf());

	// Bricks texture - normal
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Bricks_n.png",
		0,
		bricksTextureN.GetAddressOf());

	// Bricks texture - roughness
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Bricks_r.png",
		0,
		bricksTextureR.GetAddressOf());

	// Planks texture
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Planks.png",
		0,
		planksTexture.GetAddressOf());

	// Planks texture - normal
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Planks_n.png",
		0,
		planksTextureN.GetAddressOf());

	// Planks texture - roughness
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Planks_r.png",
		0,
		planksTextureR.GetAddressOf());

	// Rocks texture
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Rocks.png",
		0,
		rocksTexture.GetAddressOf());

	// Rocks texture - normal
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Rocks_n.png",
		0,
		rocksTextureN.GetAddressOf());

	// Rocks texture - roughness
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Rocks_r.png",
		0,
		rocksTextureR.GetAddressOf());

	// Stones texture
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Stones.png",
		0,
		stonesTexture.GetAddressOf());

	// Stones texture - normal
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Stones_n.png",
		0,
		stonesTextureN.GetAddressOf());

	// Stones texture - roughness
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/Stones_r.png",
		0,
		stonesTextureR.GetAddressOf());
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

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

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

		// FLOAT3 Tangent
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			4,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}

	// Describe the sampler state used for textures
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // X wrapping
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // Y wrapping
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // Z wrapping (3D textures only)
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16; // Can set this value high for now because it shouldn't impact performance that much
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Mipmapping at any range

	// Create the sampler
	Graphics::Device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	/* Finally, create the materials. These will store the
	 * shaders that were just loaded */
	materials = {
		// Bricks
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
		// Planks
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
		// Rocks
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)),
		// Stones
		std::make_shared<Material>(vertexShader, pixelShader, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
	};

	// Assign textures
	{
		materials[0]->AddTexture(0, bricksTexture);
		materials[0]->AddTexture(1, bricksTextureN);
		materials[0]->AddTexture(2, bricksTextureR);
		materials[0]->AddSampler(0, sampler);

		materials[1]->AddTexture(0, planksTexture);
		materials[1]->AddTexture(1, planksTextureN);
		materials[1]->AddTexture(2, planksTextureR);
		materials[1]->AddSampler(0, sampler);

		materials[2]->AddTexture(0, rocksTexture);
		materials[2]->AddTexture(1, rocksTextureN);
		materials[2]->AddTexture(2, rocksTextureR);
		materials[2]->AddSampler(0, sampler);

		materials[3]->AddTexture(0, stonesTexture);
		materials[3]->AddTexture(1, stonesTextureN);
		materials[3]->AddTexture(2, stonesTextureR);
		materials[3]->AddSampler(0, sampler);
	}
}


// --------------------------------------------------------
// Creates entities to be rendered
// --------------------------------------------------------
void Game::CreateEntities()
{
	unsigned int gridWidth = (unsigned int)meshes.size();
	unsigned int gridHeight = 1;
	float gridSpacing = 3.0f;
	float gridXOffset = -(float)(gridWidth - 1) * gridSpacing * 0.5f;
	float gridYOffset = -(float)(gridHeight - 1) * gridSpacing * 0.5f;

	/* Since the entity list is being auto-generated, we need some
	 * way to define which one uses which material */
	unsigned int materialIndices[] = {
		0, 1, 1, 2, 2, 2, 3
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
// Initializes data for lights
// --------------------------------------------------------
void Game::CreateLights()
{
	lightAmbient = XMFLOAT4(0.25f, 0.375f, 0.5f, 1.0f);

	// Create first light
	{
		Light newLight = {};
		newLight.Type = LIGHT_TYPE_DIRECTIONAL;
		newLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		newLight.Intensity = 0.5f;
		// Set light direction (must be normalized)
		XMVECTOR vDir = XMVectorSet(-0.1f, -1.0f, 0.5f, 0.0f);
		vDir = XMVector3Normalize(vDir);
		XMStoreFloat3(&newLight.Direction, vDir);
		// Add light to list
		lights.push_back(newLight);
	}

	// Create second light
	{
		Light newLight = {};
		newLight.Type = LIGHT_TYPE_DIRECTIONAL;
		newLight.Color = XMFLOAT3(1.0f, 0.95f, 0.9f);
		newLight.Intensity = 1.0f;
		// Set light direction (must be normalized)
		XMVECTOR vDir = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		vDir = XMVector3Normalize(vDir);
		XMStoreFloat3(&newLight.Direction, vDir);
		// Add light to list
		lights.push_back(newLight);
	}

	// Create third light
	{
		Light newLight = {};
		newLight.Type = LIGHT_TYPE_POINT;
		newLight.Color = XMFLOAT3(1.0f, 0.25f, 0.25f);
		newLight.Intensity = 2.0f;
		newLight.Position = XMFLOAT3(-1.5f, 0.0f, 0.0f);
		newLight.Range = 6.0f;
		// Add light to list
		lights.push_back(newLight);
	}

	// Create fourth light
	{
		Light newLight = {};
		newLight.Type = LIGHT_TYPE_POINT;
		newLight.Color = XMFLOAT3(0.25f, 0.25f, 1.0f);
		newLight.Intensity = 2.0f;
		newLight.Position = XMFLOAT3(1.5f, 0.0f, 0.0f);
		newLight.Range = 6.0f;
		// Add light to list
		lights.push_back(newLight);
	}

	// Create fifth light
	{
		Light newLight = {};
		newLight.Type = LIGHT_TYPE_SPOT;
		newLight.Color = XMFLOAT3(0.25f, 1.0f, 0.25f);
		newLight.Intensity = 1.5f;
		newLight.Position = XMFLOAT3(6.0f, 1.0f, 0.0f);
		newLight.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
		newLight.Range = 8.0f;
		newLight.SpotInnerAngle = XM_PI * 0.2f;
		newLight.SpotOuterAngle = XM_PI * 0.25f;
		// Add light to list
		lights.push_back(newLight);
	}
}


// --------------------------------------------------------
// Initialize sky data and load the sky texture
// --------------------------------------------------------
void Game::CreateSky()
{
	// Load sky shaders
	ID3DBlob* skyVertexShaderBlob = LoadShaderBlob(L"SkyVertex.cso");
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVertexShader = LoadVertexShader(skyVertexShaderBlob);

	ID3DBlob* skyPixelShaderBlob = LoadShaderBlob(L"SkyPixel.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPixelShader = LoadPixelShader(skyPixelShaderBlob);

	// Describe the sampler state used for the sky
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // X wrapping
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // Y wrapping
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // Z wrapping (3D textures only)
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16; // Can set this value high for now because it shouldn't impact performance that much
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Mipmapping at any range

	// Create the sampler
	Graphics::Device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	// Create the sky object
	sky = std::make_shared<Sky>(meshes[0], skyVertexShader, skyPixelShader, sampler);
	// Load the texture into the sky
	sky->SetTexture(sky->CreateCubemap(
		L"Assets/Textures/SkyCloudsBlue/Right.png",
		L"Assets/Textures/SkyCloudsBlue/Left.png",
		L"Assets/Textures/SkyCloudsBlue/Up.png",
		L"Assets/Textures/SkyCloudsBlue/Down.png",
		L"Assets/Textures/SkyCloudsBlue/Front.png",
		L"Assets/Textures/SkyCloudsBlue/Back.png"));
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

	// Continuously rotate entities for testing
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->Rotate(0.0f, deltaTime * 0.5f, 0.0f);
	}

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
		float clearColor[] = {0.0f, 0.0f, 0.0f};

		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), clearColor);
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

	// Draw the sky after geometry to avoid overdraw
	sky->Draw(cameras[activeCameraIndex].get());

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
	entity->GetMaterial()->BindShaders();

	// Update vertex shader constant buffer values
	{
		// Set up the buffer data struct
		VertexShaderConstData externalData = {};
		externalData.world = entity->GetTransform()->GetWorldMatrix();
		externalData.worldInvTranspose = entity->GetTransform()->GetWorldInverseTransposeMatrix();
		externalData.view = cameras[activeCameraIndex]->GetViewMatrix();
		externalData.projection = cameras[activeCameraIndex]->GetProjectionMatrix();

		Graphics::FillAndBindNextConstantBuffer(
			&externalData,
			sizeof(externalData),
			D3D11_VERTEX_SHADER,
			0);
	}

	// Update pixel shader constant buffer values
	{
		// Set up the buffer data struct
		PixelShaderConstData externalData = {};
		externalData.textureScale = entity->GetMaterial()->GetTextureScale();
		externalData.textureOffset = entity->GetMaterial()->GetTextureOffset();
		externalData.tint = entity->GetMaterial()->GetTint();
		externalData.cameraPosition = cameras[activeCameraIndex]->GetTransform()->GetPosition();
		externalData.time = totalTime;
		externalData.lightAmbient = lightAmbient;

		// Fill out as many lights as possible
		for (unsigned int i = 0; i < lights.size() && i < 5; i++)
		{
			memcpy(&externalData.lights[i], &lights[i], sizeof(Light));
		}

		Graphics::FillAndBindNextConstantBuffer(
			&externalData,
			sizeof(externalData),
			D3D11_PIXEL_SHADER,
			0);
	}

	entity->GetMaterial()->BindTexturesAndSamplers();

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

	// Show a panel for displaying debug information on the materials
	if (ImGui::TreeNode("Materials"))
	{
		for (unsigned int i = 0; i < materials.size(); i++)
		{
			BuildMaterialUI(materials[i].get(), i);
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

	// Show a panel for modifying light data
	if (ImGui::TreeNode("Lights"))
	{
		// Color picker for lighting ambient color
		ImGui::ColorEdit4("Ambient color", &lightAmbient.x);

		for (unsigned int i = 0; i < lights.size(); i++)
		{
			BuildLightUI(&lights[i], i);
		}

		ImGui::TreePop();
	}

	ImGui::End(); // Ends the current window
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


// Build a UI to display debug information about a material
void Game::BuildMaterialUI(Material* material, int index)
{
	if (!ImGui::TreeNode(std::format("Material {}", index).c_str()))
		return;

	DirectX::XMFLOAT2 v = material->GetTextureScale();
	ImGui::DragFloat2("Texture Scale", &v.x, 0.1f);
	material->SetTextureScale(v);

	v = material->GetTextureOffset();
	ImGui::DragFloat2("Texture Offset", &v.x, 0.1f);
	material->SetTextureOffset(v);

	DirectX::XMFLOAT4 tint = material->GetTint();
	ImGui::ColorEdit4("Tint", &tint.x);
	material->SetTint(tint);

	// Provide a preview of each texture
	for (auto& pair : material->GetTextures())
	{
		ImGui::Image(ImTextureRef((void*)pair.second.Get()), ImVec2(128.0f, 128.0f));
	}

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
	// DragFloat3 might modify the value
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


// Build a UI to control light data
void Game::BuildLightUI(Light* light, int index)
{
	if (!ImGui::TreeNode(std::format("Light {}", index).c_str()))
		return;

	ImGui::ColorEdit3("Color", &light->Color.x);
	ImGui::DragFloat("Intensity", &light->Intensity, 0.1f);

	ImGui::DragFloat3("Position", &light->Position.x, 0.1f);
	ImGui::DragFloat3("Direction", &light->Direction.x, 0.1f);
	ImGui::DragFloat("Range", &light->Range, 0.1f);

	ImGui::DragFloat("Spot Inner Angle", &light->SpotInnerAngle, 0.1f);
	ImGui::DragFloat("Spot Outer Angle", &light->SpotOuterAngle, 0.1f);

	ImGui::TreePop();
}
