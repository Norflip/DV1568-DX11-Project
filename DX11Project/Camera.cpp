#include "Camera.h"

Camera::Camera(float fov, size_t width, size_t height) : fov(fov), width(width), height(height), position(dx::XMVectorZero()), rotation(dx::XMVectorZero()), terrain(nullptr)
{
	this->aspect = static_cast<float>(width) / static_cast<float>(height);
	this->projection = DirectX::XMMatrixPerspectiveFovLH(fov * 0.017453f /* rad */, aspect, 0.0001f, 1000.0f);
}

Camera::~Camera()
{
}

void Camera::UpdateViewMatrix()
{
	dx::XMMATRIX rot = dx::XMMatrixRotationRollPitchYawFromVector(rotation);
	dx::XMVECTOR up = DirectX::XMVector3TransformNormal({ 0,1,0 }, rot);
	dx::XMVECTOR forward = dx::XMVector3Transform({ 0,0,1 }, rot);
	this->view = DirectX::XMMatrixLookToLH(position, DirectX::XMVector3Normalize(forward), DirectX::XMVector3Normalize(up));
}

Ray Camera::ScreenToRay(dx::XMFLOAT2 mousePosition) const
{
	dx::XMVECTOR cScreenSpace = dx::XMVectorSet(mousePosition.x, mousePosition.y, 0.0f, 0.0f);
	dx::XMVECTOR cObjectSpace = dx::XMVector3Unproject(cScreenSpace, 0, 0, (float)width, (float)height, 0.0f, 1.0f, projection, view, dx::XMMatrixIdentity());

	dx::XMVECTOR dir;
	dir = dx::XMVectorSubtract(cObjectSpace, position);
	dir = dx::XMVector3Normalize(dir);

	return Ray(position, dir);
}
