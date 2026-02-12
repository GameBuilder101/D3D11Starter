#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh)
{
	transform = Transform();
	this->mesh = mesh;
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

void Entity::Draw()
{
	mesh->Draw();
}
