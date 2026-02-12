#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"

// Stores and manipulates data for each entity
class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh);
	~Entity();
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	Transform* GetTransform();
	std::shared_ptr<Mesh> GetMesh();

	// Draws the associated mesh
	void Draw();

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
};