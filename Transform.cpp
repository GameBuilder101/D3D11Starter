#include "Transform.h"

using namespace DirectX;

Transform::Transform()
{
	position = XMFLOAT3();
	rotation = XMFLOAT3();
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	dirty = false;

	// Quickly fill out matrices with identity values
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

Transform::~Transform() {}

Transform::Transform(const Transform& other)
{
	position = other.position;
	rotation = other.rotation;
	scale = other.scale;

	dirty = other.dirty;

	world = other.world;
	worldInverseTranspose = other.worldInverseTranspose;
}

Transform& Transform::operator=(const Transform& other)
{
	position = other.position;
	rotation = other.rotation;
	scale = other.scale;

	dirty = other.dirty;

	world = other.world;
	worldInverseTranspose = other.worldInverseTranspose;

	return *this;
}

XMFLOAT3 Transform::GetPosition()
{
	return position;
}

XMFLOAT3 Transform::GetRotation()
{
	return rotation;
}

XMFLOAT3 Transform::GetScale()
{
	return scale;
}

void Transform::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3(x, y, z));
}

void Transform::SetPosition(XMFLOAT3 position)
{
	this->position = position;
	dirty = true; // Mark dirty to recalculate matrix when actually needed
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(XMFLOAT3(pitch, yaw, roll));
}

void Transform::SetRotation(XMFLOAT3 pitchYawRoll)
{
	this->rotation = pitchYawRoll;
	dirty = true; // Mark dirty to recalculate matrix when actually needed
}

void Transform::SetScale(float x, float y, float z)
{
	SetScale(XMFLOAT3(x, y, z));
}

void Transform::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
	dirty = true; // Mark dirty to recalculate matrix when actually needed
}

// Returns the calculated world matrix
XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateWorldMatrices();
	return world;
}

// Returns the calculated world inverse transpose matrix
XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateWorldMatrices();
	return worldInverseTranspose;
}

// Recalculates both the world and worldInverseTranspose matrices
void Transform::UpdateWorldMatrices()
{
	if (!dirty)
		return;
	dirty = false;

	XMMATRIX mTranslation = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX mRotation = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX mScale = XMMatrixScaling(scale.x, scale.y, scale.z);

	// Apply in this order for most predictable results
	XMMATRIX mWorld = mScale * mRotation * mTranslation;

	// Store calculated matrix
	XMStoreFloat4x4(&world, mWorld);
	XMStoreFloat4x4(&worldInverseTranspose,
		XMMatrixInverse(0, XMMatrixTranspose(mWorld)));
}

XMFLOAT3 Transform::GetRight()
{
	XMFLOAT3 dir = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMVECTOR vDir = XMLoadFloat3(&dir);
	XMVECTOR qRotation = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	vDir = XMVector3Rotate(vDir, qRotation);
	XMStoreFloat3(&dir, vDir);
	return dir;
}

XMFLOAT3 Transform::GetUp()
{
	XMFLOAT3 dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR vDir = XMLoadFloat3(&dir);
	XMVECTOR qRotation = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	vDir = XMVector3Rotate(vDir, qRotation);
	XMStoreFloat3(&dir, vDir);
	return dir;
}

XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR vDir = XMLoadFloat3(&dir);
	XMVECTOR qRotation = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	vDir = XMVector3Rotate(vDir, qRotation);
	XMStoreFloat3(&dir, vDir);
	return dir;
}

// Moves without taking orientation into account
void Transform::MoveAbsolute(float x, float y, float z)
{
	MoveAbsolute(XMFLOAT3(x, y, z));
}

// Moves without taking orientation into account
void Transform::MoveAbsolute(XMFLOAT3 offset)
{
	// Add with SIMD
	XMVECTOR vPosition = XMLoadFloat3(&position);
	XMVECTOR vOffset = XMLoadFloat3(&offset);
	XMStoreFloat3(&position, vPosition + vOffset);
	dirty = true;
}

// Moves relative to local forward
void Transform::MoveRelative(float x, float y, float z)
{
	MoveRelative(XMFLOAT3(x, y, z));
}

// Moves relative to local forward
void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	// Calculate with SIMD
	XMVECTOR vOffset = XMLoadFloat3(&offset);
	XMVECTOR qRotation = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	// Rotate the offset vector by the rotation quaternion
	vOffset = XMVector3Rotate(vOffset, qRotation);

	XMVECTOR vPosition = XMLoadFloat3(&position);
	XMStoreFloat3(&position, vPosition + vOffset);
	dirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	Rotate(XMFLOAT3(pitch, yaw, roll));
}

void Transform::Rotate(XMFLOAT3 pitchYawRoll)
{
	// Add with SIMD
	XMVECTOR vRotation = XMLoadFloat3(&rotation);
	XMVECTOR vOffset = XMLoadFloat3(&pitchYawRoll);
	XMStoreFloat3(&rotation, vRotation + vOffset);
	dirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	Scale(XMFLOAT3(x, y, z));
}

void Transform::Scale(XMFLOAT3 scale)
{
	// Multiply with SIMD
	XMVECTOR vScale = XMLoadFloat3(&(this->scale));
	XMVECTOR vMult = XMLoadFloat3(&scale);
	XMStoreFloat3(&(this->scale), vScale * vMult);
	dirty = true;
}
