#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

// Stores and manipulates data for each entity
class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Entity();
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	Transform* GetTransform();

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();

private:
	Transform transform;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};