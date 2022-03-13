#include "Shader.h"

ID3D11PixelShader* pixelShader;
ID3D11GeometryShader* geometryShader;
ID3D11VertexShader* vertexShader;
ID3D11InputLayout* inputLayout;

Shader::Shader() : vs(L""), ps(L""), gs(L""), pixelShader(nullptr), geometryShader(nullptr), vertexShader(nullptr), inputLayout(nullptr)
{
	flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	flags = flags | D3DCOMPILE_DEBUG;
#endif
}

Shader::~Shader()
{
}

void Shader::Load(ID3D11Device* device, bool isParticle)
{
	LoadPS(device);
	LoadGS(device);
	LoadVS(device, isParticle);
}

void Shader::Bind(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(inputLayout);
	context->VSSetShader(vertexShader, 0, 0);
	context->GSSetShader(geometryShader, 0, 0);
	context->PSSetShader(pixelShader, 0, 0);
}

void Shader::LoadPS(ID3D11Device* device)
{
	if (pixelShader != nullptr)
	{
		pixelShader->Release();
		pixelShader = nullptr;
	}
	
	ID3DBlob* errorBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	HRESULT ps_result = D3DCompileFromFile(ps, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, &psBlob, &errorBlob);

	if (FAILED(ps_result))
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	ps_result = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
	assert(SUCCEEDED(ps_result));
}

void Shader::LoadGS(ID3D11Device* device)
{
	if (gs != L"")
	{
		if (geometryShader != nullptr)
		{
			geometryShader->Release();
			geometryShader = nullptr;
		}

		ID3DBlob* gsBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT gs_result = D3DCompileFromFile(gs, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "gs_5_0", flags, 0, &gsBlob, &errorBlob);

		if (FAILED(gs_result))
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());

		gs_result = device->CreateGeometryShader(gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &geometryShader);
		assert(SUCCEEDED(gs_result));
	}
}

void Shader::LoadVS(ID3D11Device* device, bool isParticle)
{
	D3D11_INPUT_ELEMENT_DESC DX11_INPUT_LAYOUT[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	 0, D3D11_INPUT_PER_VERTEX_DATA, 0},		// 0 + 12 b = 12 b
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,	12, D3D11_INPUT_PER_VERTEX_DATA, 0},		// 12 + 8 b = 20 b		
		{"NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	20, D3D11_INPUT_PER_VERTEX_DATA, 0},		// 20 + 12 b = 32b
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}			// 32 + 12 b = 44b
	};

	D3D11_INPUT_ELEMENT_DESC DX11_INPUT_LAYOUT_PARTICLE[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,	 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	if (vertexShader != nullptr)
	{
		vertexShader->Release();
		vertexShader = nullptr;
	}

	if (inputLayout != nullptr)
	{
		inputLayout->Release();
		inputLayout = nullptr;
	}

	ID3DBlob* errorBlob = nullptr;
	ID3DBlob* vsBlob = nullptr;
	HRESULT vs_result = D3DCompileFromFile(vs, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", flags, 0, &vsBlob, &errorBlob);

	if (FAILED(vs_result))
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	vs_result = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
	assert(SUCCEEDED(vs_result));

	if (isParticle)
		vs_result = device->CreateInputLayout(DX11_INPUT_LAYOUT_PARTICLE, ARRAYSIZE(DX11_INPUT_LAYOUT_PARTICLE), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
	else
		vs_result = device->CreateInputLayout(DX11_INPUT_LAYOUT, ARRAYSIZE(DX11_INPUT_LAYOUT), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

	assert(SUCCEEDED(vs_result));

	if (errorBlob != nullptr)
		errorBlob->Release();
}
