#pragma once

#include <DirectXMath.h>

// Stores and converts position/rotation/scale data into a single matrix
class Transform
{
public:
	Transform();
	~Transform();
	Transform(const Transform&);
	Transform& operator=(const Transform&);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();

	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 pitchYawRoll);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);

	// Returns the calculated world matrix
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	// Returns the calculated world inverse transpose matrix
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();

	// Moves without taking orientation into account
	void MoveAbsolute(float x, float y, float z);
	// Moves without taking orientation into account
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	// Moves relative to local forward
	void MoveRelative(float x, float y, float z);
	// Moves relative to local forward
	void MoveRelative(DirectX::XMFLOAT3 offset);

	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 pitchYawRoll);

	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);

private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation; // In the form of pitch, yaw, roll
	DirectX::XMFLOAT3 scale;

	/* Gets set to true when a value is modified. When GetWorldMatrix
	 * is next called, it will recalculate the matrix */
	bool dirty;

	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInverseTranspose;

	// Recalculates both the world and worldInverseTranspose matrices
	void UpdateWorldMatrices();
};