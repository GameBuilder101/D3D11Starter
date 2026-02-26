#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

// Is able to create and store buffers for mesh data
class Mesh
{
public:
	Mesh(Vertex* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);
	Mesh(const char* filePath);
	~Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetVertexBufferCount();
	unsigned int GetIndexBufferCount();

	// Sets the buffers and draws the correct number of vertices
	void Draw();

private:
	void CreateBuffers(Vertex* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);

	// Vertex/index buffers specific to this mesh
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int vertexBufferCount;
	unsigned int indexBufferCount;
};
