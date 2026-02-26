#include "Camera.h"

using namespace DirectX;

Camera::Camera(float aspectRatio, XMFLOAT3 position, XMFLOAT3 pitchYawRoll, float fieldOfView)
{
	// Set initial position
	transform = Transform();
	transform.SetPosition(position);
	transform.SetRotation(pitchYawRoll);

	UpdateViewMatrix();
	UpdateProjectionMatrixFull(aspectRatio, fieldOfView, 0.1f, 1000.0f);

	moveSpeed = 8.0f;
	lookSpeed = 0.005f;
}

Camera::~Camera() {}

Transform* Camera::GetTransform()
{
	return &transform;
}

float Camera::GetFOV()
{
	return fieldOfView;
}

float Camera::GetAspectRatio()
{
	return aspectRatio;
}

float Camera::GetNearPlane()
{
	return nearPlane;
}

float Camera::GetFarPlane()
{
	return farPlane;
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return view;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projection;
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR vPosition = XMLoadFloat3(&position);
	XMVECTOR vForward = XMLoadFloat3(&forward);
	XMVECTOR vWorldUp = XMLoadFloat3(&worldUp);
	XMMATRIX mView = XMMatrixLookToLH(vPosition, vForward, vWorldUp);
	XMStoreFloat4x4(&view, mView);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	UpdateProjectionMatrixFull(aspectRatio, fieldOfView, nearPlane, farPlane);
}

void Camera::UpdateProjectionMatrixFull(float aspectRatio, float fieldOfView, float nearPlane, float farPlane)
{
	this->fieldOfView = fieldOfView;
	this->aspectRatio = aspectRatio;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;

	XMMATRIX mProjection = XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearPlane, farPlane);
	XMStoreFloat4x4(&projection, mProjection);
}

void Camera::Update(float deltaTime)
{
	float dMoveSpeed = moveSpeed * deltaTime;
	float dLookSpeed = lookSpeed * deltaTime;

	// Store relative movements so they can be normalized before getting applied
	XMFLOAT3 moveRelative = XMFLOAT3();
	XMVECTOR vMoveRelative = XMLoadFloat3(&moveRelative);

	// Forward/back input
	if (Input::KeyDown('W'))
	{
		// Move forward relative
		vMoveRelative += XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}
	else if (Input::KeyDown('S'))
	{
		// Move backward relative
		vMoveRelative += XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	}

	// Left/right input
	if (Input::KeyDown('A'))
	{
		// Move left relative
		vMoveRelative += XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
	}
	else if (Input::KeyDown('D'))
	{
		// Move right relative
		vMoveRelative += XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}

	// Normalize movement inputs
	vMoveRelative = XMVector3Normalize(vMoveRelative);
	vMoveRelative *= dMoveSpeed;
	XMStoreFloat3(&moveRelative, vMoveRelative);

	// Perform relative movement if an input was pressed
	if (moveRelative.x != 0.0f || moveRelative.y != 0.0f || moveRelative.z != 0.0f)
	{
		transform.MoveRelative(moveRelative);
	}

	// Up/down input
	if (Input::KeyDown(VK_SPACE))
	{
		// Move up absolute
		transform.MoveAbsolute(0.0f, dMoveSpeed, 0.0f);
	}
	else if (Input::KeyDown(VK_CONTROL))
	{
		// Move down absolute
		transform.MoveAbsolute(0.0f, -dMoveSpeed, 0.0f);
	}

	// Handle look inputs
	if (Input::MouseRightDown())
	{
		float mouseXDelta = Input::GetMouseXDelta() * lookSpeed;
		float mouseYDelta = Input::GetMouseYDelta() * lookSpeed;
		transform.Rotate(mouseYDelta, mouseXDelta, 0.0f);

		// Get the new rotation for clamping purposes
		XMFLOAT3 rotation = transform.GetRotation();

		// Clamp the up/down look rotation to prevent flipping upside-down
		float limit = XM_PI * 0.499f;
		if (rotation.x < -limit)
			rotation.x = -limit;
		else if (rotation.x > limit)
			rotation.x = limit;

		transform.SetRotation(rotation);
	}

	// Update to match new transform
	UpdateViewMatrix();
}
