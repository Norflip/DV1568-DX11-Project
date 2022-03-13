#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Terrain.h"
#include "Ray.h"
#include <DirectXMath.h>
namespace dx = DirectX;

class Camera
{
public:
	Camera(float fov, size_t width, size_t height);
	virtual ~Camera();

	dx::XMMATRIX GetViewMatrix() { return this->view; }
	dx::XMMATRIX GetProjectionMatrix() { return this->projection; }
	void SetTerrain(Terrain* terrain) { this->terrain = terrain; }
	void UpdateViewMatrix();

	dx::XMVECTOR position;
	dx::XMVECTOR rotation;

	Ray ScreenToRay(dx::XMFLOAT2 mousePosition) const;

private:
	dx::XMMATRIX projection;
	dx::XMMATRIX view;

	size_t width, height;
	float fov, aspect;

	Terrain* terrain;
};