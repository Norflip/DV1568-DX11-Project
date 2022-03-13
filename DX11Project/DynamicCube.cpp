#include "DynamicCube.h"

DynamicCube::DynamicCube()
{

}

DynamicCube::~DynamicCube()
{
}

void DynamicCube::Initialize(DX11* dx11)
{
	this->dx11 = dx11;

	Shader* shader = new Shader();
	shader->SetVSPath(L"DynamicCube_vs.hlsl");
	shader->SetGSPath(L"BackfaceCulling_gs.hlsl");
	shader->SetPSPath(L"DynamicCube_ps.hlsl");
	shader->Load(dx11->device);

	Model* model = OBJLoader::LoadOBJ("Models/sphere.obj", shader, dx11);
	SetMesh(model->GetMesh());
	SetMaterial(model->GetMaterial());
	delete model;

	SetupCameras();
	gBuffer = dx11->CreateRenderTargets(3, DYNAMIC_RESOLUTION, DYNAMIC_RESOLUTION);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(DYNAMIC_RESOLUTION);
	viewport.Height = static_cast<float>(DYNAMIC_RESOLUTION);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = DYNAMIC_RESOLUTION;
	texDesc.Height = DYNAMIC_RESOLUTION;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	texDesc.CPUAccessFlags = 0;

	ID3D11Texture2D* cubeTexture;
	HRESULT hr = dx11->device->CreateTexture2D(&texDesc, 0, &cubeTexture);
	assert(SUCCEEDED(hr));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;

	//rtv = new ID3D11RenderTargetView * [6];

	for (size_t i = 0; i < 6; i++)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = dx11->device->CreateRenderTargetView(cubeTexture, &rtvDesc, &rtv[i]);
		assert(SUCCEEDED(hr));
	}



	// SHADER RESOURC
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	hr = dx11->device->CreateShaderResourceView(cubeTexture, &srvDesc, &srv);
	assert(SUCCEEDED(hr));

	cubeTexture->Release();

	// DEPTH TEXTURE
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = DYNAMIC_RESOLUTION;
	depthTexDesc.Height = DYNAMIC_RESOLUTION;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	hr = dx11->device->CreateTexture2D(&depthTexDesc, 0, &depthTex);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = dx11->device->CreateDepthStencilView(depthTex, &dsvDesc, &dsv);
	assert(SUCCEEDED(hr));

	depthTex->Release();

	GetMaterial()->AddTexture(srv, TextureType::Pixel);
	GetMaterial()->AddSampler(dx11->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP), TextureType::Pixel);
}

void DynamicCube::UpdateCameraPosition()
{
	for (size_t i = 0; i < 6; i++)
	{
		cameras[i]->position = this->position;
		cameras[i]->UpdateViewMatrix();
	}
}

RenderTarget DynamicCube::GetRenderTarget(size_t i)
{
	RenderTarget tmpRenderTarget;
	tmpRenderTarget.count = 1;
	tmpRenderTarget.srv = &srv;
	tmpRenderTarget.dsv = dsv;
	tmpRenderTarget.viewport = gBuffer.viewport;
	tmpRenderTarget.rtv = &rtv[i];
	return tmpRenderTarget;
}

Camera* DynamicCube::GetCamera(size_t i)
{
	return this->cameras[i];
}

void DynamicCube::SetupCameras()
{
	float st = 90 * 0.0174532f;
	dx::XMFLOAT3 rotations[6]
	{
		{0,st,0},
		{0,-st,0},
		{-st,0,0},
		{st,0,0},
		{0,0,0},
		{0,st * 2,0},
	};

	for (size_t i = 0; i < 6; i++)
	{
		cameras[i] = new Camera(90, DYNAMIC_RESOLUTION, DYNAMIC_RESOLUTION);
		cameras[i]->position = this->position;
		cameras[i]->rotation = dx::XMLoadFloat3(&rotations[i]);
		cameras[i]->UpdateViewMatrix();
	}
}
