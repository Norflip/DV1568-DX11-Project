#pragma once
#include <string>

#include "Mesh.h"
#include "Material.h"
#include "DX11.h"
#include <DirectXMath.h>
namespace dx = DirectX;

class Model
{
public:
	Model();
	Model(Mesh mesh, Material* material);

	virtual void Draw(ID3D11DeviceContext* context);

	dx::XMMATRIX GetModelMatrix();
	
	Material* GetMaterial() { return this->material; }
	void SetMaterial(Material* mat) { this->material = mat; }

	void SetMesh(Mesh mesh);
	Mesh& GetMesh() { return this->mesh; }

	dx::XMVECTOR position;
	dx::XMVECTOR rotation;
	bool ignorePicking;
	bool enabled;

private:
	Mesh mesh;
	Material* material;

};