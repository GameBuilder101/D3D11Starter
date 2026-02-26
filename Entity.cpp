#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	transform = Transform();
	this->mesh = mesh;
	this->material = material;
}

Entity::~Entity() {}

Transform* Entity::GetTransform()
{
	return &transform;
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return material;
}
