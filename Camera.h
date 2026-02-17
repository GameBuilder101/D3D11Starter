#pragma once

#include <DirectXMath.h>
#include "Transform.h"
#include "Input.h"

class Camera
{
public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 pitchYawRoll, float fieldOfView = 45.0f);
	~Camera();
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	Transform* GetTransform();
	float GetFOV();
	float GetAspectRatio();
	float GetNearPlane();
	float GetFarPlane();
	
	// Returns the calculated view matrix
	DirectX::XMFLOAT4X4 GetViewMatrix();
	// Returns the calculated projection matrix
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateProjectionMatrixFull(float aspectRatio, float fieldOfView, float nearPlane, float farPlane);

	void Update(float deltaTime);

private:
	Transform transform;

	float fieldOfView;
	float aspectRatio;
	float nearPlane;
	float farPlane;

	// View/projection matrices
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	float moveSpeed;
	float lookSpeed;
};