#pragma once
#include <DirectXMath.h>
namespace dx = DirectX;

// slot 0 

constexpr int WORLD_BUFFER_SLOT = 0;
__declspec(align(16))
struct WorldCBuffer
{
	dx::XMMATRIX model;
	dx::XMMATRIX view;
	dx::XMMATRIX projection;

	dx::XMFLOAT3 eye;
	float cb_w_pad0;
};


// slot 2 
constexpr int MATERIAL_BUFFER_SLOT = 1;
__declspec(align(16))
struct MaterialCBuffer
{
	dx::XMFLOAT4 ambient;
	dx::XMFLOAT4 diffuse;

	int hasAlbedoTexture;	// 4 byte
	int hasNormalTexture;  // 4 byte
	int cb_m_pad0[2];  // 8 byte

	//float cb_m_pad1[3]; // 3*4 = 12 byte
};


__declspec(align(16))
struct PointLight
{
	dx::XMFLOAT4 color;
	dx::XMFLOAT3 position;
	float radius;
};

// slot 1 
constexpr int LIGHT_BUFFER_SLOT = 2;
__declspec(align(16))
struct LightCBuffer
{
	dx::XMFLOAT4 sunColor;
	dx::XMFLOAT3 direction;
	float sunStrength;

	dx::XMFLOAT3 eye;
	int lightCount;

	PointLight lights[16];	
};

