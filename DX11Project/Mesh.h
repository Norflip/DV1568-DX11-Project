#pragma once
#include "DX11.h"
#include <vector>
#include <DirectXMath.h>
namespace dx = DirectX;

struct VERTEX
{
	dx::XMFLOAT3 position;
	dx::XMFLOAT2 uv;
	dx::XMFLOAT3 normal;
	dx::XMFLOAT3 tangent;

	VERTEX() : position(0, 0, 0), uv(0, 0), normal(0, 0, 0), tangent(0, 0, 0) {}
	VERTEX(dx::XMFLOAT3 position, dx::XMFLOAT2 uv, dx::XMFLOAT3 normal, dx::XMFLOAT3 tangent) : position(position), uv(uv), normal(normal), tangent(tangent) {}
};

struct Mesh
{
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	std::vector<VERTEX> vertexes;
	std::vector<unsigned int> indicies;

	float radius;
};