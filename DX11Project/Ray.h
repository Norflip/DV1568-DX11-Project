#pragma once
#include "Mesh.h"
#include "Model.h"
#include <DirectXMath.h>
namespace dx = DirectX;

struct RayHit
{
	bool intersected;
	dx::XMVECTOR normal;
	dx::XMVECTOR position;
	float t;

	RayHit() : intersected(false), t(0), normal({ 0,0,0 }), position({ 0,0,0 }) {}
};

class Ray
{
public:
	Ray(dx::XMVECTOR origin, dx::XMVECTOR direction);
	~Ray() {}

	dx::XMVECTOR GetOrigin() const { return this->origin; }
	dx::XMVECTOR GetDirection() const { return this->direction; }

	RayHit IntersectsModel(Model* model);
	RayHit IntersectsSphere(dx::XMVECTOR center, float radius);
	RayHit IntersectsMesh(const Mesh& mesh, dx::XMMATRIX worldMatrix) const;

private:
	RayHit IntersectsTriangle(const dx::XMVECTOR& a, const dx::XMVECTOR& b, const dx::XMVECTOR& c) const;

private:
	dx::XMVECTOR origin, direction;
	dx::XMVECTOR inv;
	int cardinalSign[3];
};
