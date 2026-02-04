#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "ConstantBuffer.h"

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
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// Create a constant buffer
	{
		// Calculate the size of the main constant buffer
		unsigned int size = sizeof(ExternalVertexData);
		size = (size + 15) / 16 * 16;

		// Describe the constant buffer
		D3D11_BUFFER_DESC cbd = {}; // Sets struct to all zeros
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.ByteWidth = size; // Must be a multiple of 16
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.Usage = D3D11_USAGE_DYNAMIC;

		Graphics::Device->CreateBuffer(&cbd, 0, constBuffer.GetAddressOf());
		// Bind the buffer to register 0 for now
		Graphics::Context->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());
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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Homogeneous screen coords for first test mesh (triangle)
	Vertex triVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};
	unsigned int triIndices[] = { 0, 1, 2 };
	// Make the test mesh
	testTriMesh = std::make_shared<Mesh>(
		triVertices,
		triIndices,
		3,
		3);

	// Homogeneous screen coords for second test mesh (square)
	Vertex quadVertices[] =
	{
		{ XMFLOAT3(+0.7f, +0.7f, +0.0f), blue },
		{ XMFLOAT3(+0.7f, +0.9f, +0.0f), green },
		{ XMFLOAT3(+0.9f, +0.9f, +0.0f), green },
		{ XMFLOAT3(+0.9f, +0.7f, +0.0f), blue },
	};
	unsigned int quadIndices[] = { 0, 1, 2, 2, 3, 0 };
	// Make the test mesh
	testQuadMesh = std::make_shared<Mesh>(
		quadVertices,
		quadIndices,
		4,
		6);
	
	// Homogeneous screen coords for third test mesh (pentagon)
	Vertex pentagonVertices[] =
	{
		{ XMFLOAT3(-0.7f, -0.7f, +0.0f), blue },
		{ XMFLOAT3(-0.75f, -0.875f, +0.0f), red },
		{ XMFLOAT3(-0.85f, -0.875f, +0.0f), red },
		{ XMFLOAT3(-0.9f, -0.7f, +0.0f), green },
		{ XMFLOAT3(-0.8f, -0.55f, +0.0f), red },
	};
	unsigned int pentagonIndices[] = { 0, 1, 2, 2, 3, 0, 0, 3, 4 };
	// Make the test mesh
	testPentagonMesh = std::make_shared<Mesh>(
		pentagonVertices,
		pentagonIndices,
		5,
		9);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	UpdateImGui(deltaTime, totalTime);
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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

	// Show a panel for displaying debug information on the test meshes
	if (ImGui::CollapsingHeader("Meshes"))
	{
		BuildMeshUI(testTriMesh.get(), "Mesh: Triangle");
		BuildMeshUI(testQuadMesh.get(), "Mesh: Quad");
		BuildMeshUI(testPentagonMesh.get(), "Mesh: Pentagon");
	}

	// Show a panel for modifying external vertex data
	if (ImGui::CollapsingHeader("Constant Buffer Vertex Data"))
	{
		ImGui::ColorEdit4("Color Tint", vertexColorTint);
		ImGui::DragFloat3("Offset", vertexOffset);
	}

	// Test a toggle header
	/*if (ImGui::CollapsingHeader("Test toggle header"))
	{
		// Test a text box
		ImGui::InputText("Text text box", testTextBox, 128);

		// Test a slider
		ImGui::SliderInt("Test slider", &testSlider, 0, 100);
	}*/

	ImGui::End(); // Ends the current window

	// Show the demo window
	if (showDemoWindow)
		ImGui::ShowDemoWindow();
}


// Build a UI to display debug information about a mesh
void Game::BuildMeshUI(Mesh* mesh, const char name[])
{
	if (!ImGui::CollapsingHeader(name))
		return;

	ImGui::Text("Triangles: %d", mesh->GetIndexBufferCount() / 3);
	ImGui::Text("Vertices: %d", mesh->GetVertexBufferCount());
	ImGui::Text("Indices: %d", mesh->GetIndexBufferCount());
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

	// Upload CPU data to GPU constant buffers
	{
		ExternalVertexData externalData;
		externalData.colorTint = XMFLOAT4(vertexColorTint);
		externalData.offset = XMFLOAT3(vertexOffset);

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		// Lock memory
		Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		// Copy into memory
		memcpy(mappedBuffer.pData, &externalData, sizeof(externalData));
		// Unlock memory
		Graphics::Context->Unmap(constBuffer.Get(), 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	{
		testTriMesh->Draw();
		testQuadMesh->Draw();
		testPentagonMesh->Draw();
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
