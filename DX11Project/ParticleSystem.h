#pragma once
#include "Model.h"
#include "Shader.h"
#include "Mesh.h"
#include <string>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include <DirectXMath.h>
namespace dx = DirectX;

class ParticleSystem : public Model
{
	const LPCWSTR PATH = L"Particle_cs.hlsl";
	const int MAX_PARTICLES = 1000;
	const float MAX_LIFETIME = 5.0f;

	struct ParticleData
	{
		dx::XMFLOAT3 startPosition;
		dx::XMFLOAT3 velocity;
		float gravityModifier;
		float timeOffset;
		dx::XMFLOAT4 startColor;
		dx::XMFLOAT4 endColor;
	};

	struct Particle
	{
		dx::XMFLOAT3 position;
		dx::XMFLOAT4 color;
	};

	__declspec(align(16))
	struct EmitterData
	{
		dx::XMFLOAT3 emitterPosition;
		float time;

		float lifeTime;
		float pad[3];
	};

public:
	ParticleSystem();
	virtual ~ParticleSystem();

	void Initialize(DX11* dx11);
	void SetEmitterPosition(dx::XMVECTOR position);
	void UpdatePositions(const float& deltaTime);
	void Draw(ID3D11DeviceContext* context) override;

private:
	dx::XMFLOAT3 RandomPointOnSphere(float radius = 1.0f);

private:
	DX11* dx11;
	ID3D11ComputeShader* computeShader;

	EmitterData emitterData;
	ID3D11Buffer* emitterDataBuffer;

	ID3D11Buffer* particleDataBuffer;
	ID3D11Buffer* outputBuffer;
	ID3D11Buffer* vertexBuffer;

	ID3D11ShaderResourceView* inputView;
	ID3D11UnorderedAccessView* outputUAV;

	std::vector<ParticleData> particleData;
	std::vector<Particle> particlePositions;

	Shader* billboardShader;
	dx::XMVECTOR position;
};