#include "ParticleSystem.h"

ParticleSystem::ParticleSystem() : computeShader(nullptr), Model(Mesh(), nullptr), position({ 0,0,0 })
{

}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::Initialize(DX11* dx11)
{

	emitterData.time = 0.0f;
	emitterData.lifeTime = MAX_LIFETIME;
	emitterData.emitterPosition = { 0,0,0 };
	emitterDataBuffer = dx11->CreateConstantBuffer((void*)&emitterData, sizeof(emitterData));

	this->dx11 = dx11;

	DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	flags = flags | D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* csBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT gs_result = D3DCompileFromFile(PATH, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", flags, 0, &csBlob, &errorBlob);

	if (FAILED(gs_result))
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	gs_result = dx11->device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &computeShader);
	assert(SUCCEEDED(gs_result));


	this->billboardShader = new Shader();
	this->billboardShader->SetVSPath(L"Particle_vs.hlsl");
	this->billboardShader->SetGSPath(L"ParticleBillboard_gs.hlsl");
	this->billboardShader->SetPSPath(L"Particle_ps.hlsl");
	this->billboardShader->Load(dx11->device, true);

	particlePositions.clear();

	for (size_t i = 0; i < MAX_PARTICLES; i++)
	{
		Particle particle;
		particlePositions.push_back(particle);

		ParticleData pd;
		pd.startPosition = { 0,0,0 };// RandomPointOnSphere(10.0f);
		pd.velocity = RandomPointOnSphere(3.0f);
		pd.gravityModifier = 0.5f;

		// mellan 0 och max
		pd.timeOffset = (static_cast<float>(rand() % 1000) / 1000.0f) * MAX_LIFETIME;
		pd.startColor = { 0.2f, 0.9f, 0.4f,1 };
		pd.endColor = { 0.2f,0.3f,1.0f,1 };


		particleData.push_back(pd);
	}

	D3D11_BUFFER_DESC vertexBufferDescription;

	// Skapar vertex buffer
	ZeroMemory(&vertexBufferDescription, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDescription.ByteWidth = sizeof(Particle) * particlePositions.size();
	vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	//subresourceData.pSysMem = points.data();

	HRESULT vertexBufferResult = dx11->device->CreateBuffer(&vertexBufferDescription, 0, &vertexBuffer);
	assert(SUCCEEDED(vertexBufferResult));

	SetMaterial(new Material(this->billboardShader, dx11));

	HRESULT hr;

	// INPUT

	D3D11_BUFFER_DESC inputBufferDescription;
	ZeroMemory(&inputBufferDescription, sizeof(D3D11_BUFFER_DESC));
	inputBufferDescription.ByteWidth = sizeof(ParticleData) * MAX_PARTICLES;
	inputBufferDescription.StructureByteStride = sizeof(ParticleData);
	inputBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputBufferDescription.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	subresourceData.pSysMem = particleData.data();

	hr = dx11->device->CreateBuffer(&inputBufferDescription, &subresourceData, &particleDataBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = MAX_PARTICLES;
	hr = dx11->device->CreateShaderResourceView(particleDataBuffer, &srvDesc, &inputView);


	// OUTPUT
	D3D11_BUFFER_DESC outputDesc;
	ZeroMemory(&outputDesc, sizeof(D3D11_BUFFER_DESC));

	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ByteWidth = sizeof(Particle) * MAX_PARTICLES;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.StructureByteStride = sizeof(Particle);
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hr = dx11->device->CreateBuffer(&outputDesc, 0, &outputBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = MAX_PARTICLES;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	hr = dx11->device->CreateUnorderedAccessView(outputBuffer, &uavDesc, &outputUAV);
}

void ParticleSystem::SetEmitterPosition(dx::XMVECTOR position)
{
	dx::XMStoreFloat3(&emitterData.emitterPosition, position);
}

void ParticleSystem::UpdatePositions(const float& deltaTime)
{
	emitterData.time += deltaTime;
	dx11->context->UpdateSubresource(emitterDataBuffer, 0, 0, &emitterData, 0, 0);
	dx11->context->CSSetConstantBuffers(0, 1, &emitterDataBuffer);

	ID3D11DeviceContext* c = dx11->context;
	c->CSSetShader(computeShader, 0, 0);
	c->CSSetShaderResources(0, 1, &inputView);

	c->CSSetUnorderedAccessViews(0, 1, &outputUAV, 0);

	// 100 * threadgroups(10 * 1 * 1) = 1000
	c->Dispatch(100, 1, 1);

	//// Unbind the input textures from the CS for good housekeeping
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	c->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	c->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	c->CopyResource(vertexBuffer, outputBuffer);
}

void ParticleSystem::Draw(ID3D11DeviceContext* context)
{
	UINT stride = sizeof(Particle);
	UINT offset = 0;

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->Draw(particlePositions.size(), 0);
}

dx::XMFLOAT3 ParticleSystem::RandomPointOnSphere(float radius)
{
	// https://stackoverflow.com/questions/5531827/random-point-on-a-given-sphere

	const int precision = pow(10, 3);
	dx::XMFLOAT3 result;
	result.x = ((((rand() % precision) / (float)precision)) * 2) - 1;
	result.y = ((((rand() % precision) / (float)precision)) * 2) - 1;
	result.z = ((((rand() % precision) / (float)precision)) * 2) - 1;

	bool found = false;
	float sqrLength = result.x * result.x + result.y * result.y + result.z * result.z;

	if (sqrLength > 1)
	{
		result = RandomPointOnSphere(radius);
	}
	else
	{
		float length = sqrtf(sqrLength);
		result.x = (result.x / length) * radius;
		result.y = (result.y / length) * radius;
		result.z = (result.z / length) * radius;
	}

	return result;
}
