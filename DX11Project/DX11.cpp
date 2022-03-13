#include "DX11.h"

DX11::DX11() : device(nullptr), context(nullptr), swapchain(nullptr)
{
}

void DX11::Initialize(HWND& hwnd, size_t width, size_t height)
{
	/*
		DEVICE, CONTEXT & SWAPCHAIN
	*/
	DXGI_SWAP_CHAIN_DESC scDesc;
	ZeroMemory(&scDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	scDesc.BufferCount = 2;
	scDesc.BufferDesc.Width = width;
	scDesc.BufferDesc.Height = height;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	scDesc.OutputWindow = hwnd;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT swapflags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	swapflags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_0 };
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, swapflags, featureLevel, 1, D3D11_SDK_VERSION, &scDesc, &swapchain, &device, nullptr, &context);
	assert(SUCCEEDED(hr));

	D3D11_RASTERIZER_DESC rasterizerDsc;
	ZeroMemory(&rasterizerDsc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDsc.FillMode = D3D11_FILL_SOLID;
	rasterizerDsc.CullMode = D3D11_CULL_NONE;

	rasterizerDsc.DepthClipEnable = false;
	rasterizerDsc.FrontCounterClockwise = false;
	rasterizerDsc.MultisampleEnable = false;
	rasterizerDsc.ScissorEnable = false;

	hr = device->CreateRasterizerState(&rasterizerDsc, &rasterizerState);
	assert(SUCCEEDED(hr));

	context->RSSetState(rasterizerState);

	linearSampler = CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

}

void DX11::Cleanup()
{
	device->Release();
	device = nullptr;

	context->Release();
	context = nullptr;

	swapchain->Release();
	swapchain = nullptr;
}

RenderTarget DX11::CreateBackBuffer(size_t width, size_t height) const
{
	RenderTarget rt;
	rt.count = 1;
	rt.srv = nullptr;
	rt.rtv = new ID3D11RenderTargetView * [1];

	// VIEWPORT
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	rt.viewport = viewport;

	/*
		RENDER TARGET
	*/
	ID3D11Texture2D* backBufferPtr;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);

	if (backBufferPtr != nullptr)
	{
		device->CreateRenderTargetView(backBufferPtr, nullptr, rt.rtv);
		backBufferPtr->Release();
	}

	CreateDepthStencilView(rt, width, height);
	return rt;
}

RenderTarget DX11::CreateRenderTargets(size_t count, size_t width, size_t height) const
{
	RenderTarget rt;
	rt.count = count;
	rt.rtv = new ID3D11RenderTargetView * [count];
	rt.srv = new ID3D11ShaderResourceView * [count];

	// VIEWPORT
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	rt.viewport = viewport;

	// TARGET TEXTURES
	ID3D11Texture2D** targetTextures = new ID3D11Texture2D * [count];

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr;
	for (size_t i = 0; i < count; i++)
	{
		hr = device->CreateTexture2D(&textureDesc, nullptr, &targetTextures[i]);
		assert(SUCCEEDED(hr));
	}

	/*
		RENDER TARGET VIEWS
	*/
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(rtvDesc));
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// Create the render target views.
	for (size_t i = 0; i < count; i++)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;

		hr = device->CreateRenderTargetView(targetTextures[i], &rtvDesc, &rt.rtv[i]);
		assert(SUCCEEDED(hr));
	}

	/*
		SHADER RESOURCE VIEWS
	*/
	D3D11_SHADER_RESOURCE_VIEW_DESC shrDesc;
	ZeroMemory(&shrDesc, sizeof(shrDesc));
	shrDesc.Format = textureDesc.Format;
	shrDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shrDesc.Texture2D.MipLevels = 1;

	// Create the shader resource views.
	for (size_t i = 0; i < count; i++)
	{
		hr = device->CreateShaderResourceView(targetTextures[i], &shrDesc, &rt.srv[i]);
		assert(SUCCEEDED(hr));
	}

	CreateDepthStencilView(rt, width, height);

	// CLEANUP
	for (size_t i = 0; i < count; i++)
		targetTextures[i]->Release();

	delete[] targetTextures;
	return rt;
}

void DX11::CreateDepthStencilView(RenderTarget& rt, size_t width, size_t height) const
{
	ID3D11Texture2D* depthTex;

	// DEPTH TEXTURE
	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));

	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	HRESULT createT2D = device->CreateTexture2D(&depthTexDesc, 0, &depthTex);
	assert(SUCCEEDED(createT2D));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	HRESULT createDSVResult = device->CreateDepthStencilView(depthTex, &dsvDesc, &rt.dsv);
	assert(SUCCEEDED(createDSVResult));

	depthTex->Release();
	depthTex = 0;
}

ID3D11Buffer* DX11::CreateConstantBuffer(void* data, size_t size)
{
	assert((size % 16) == 0);

	ID3D11Buffer* buffer;
	ZeroMemory(&buffer, sizeof(ID3D11Buffer));

	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(D3D11_BUFFER_DESC));

	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	// WORLD BUFFER
	bufferDescription.ByteWidth = size;
	subresourceData.pSysMem = &data;
	HRESULT hr = device->CreateBuffer(&bufferDescription, &subresourceData, &buffer);
	assert(SUCCEEDED(hr));

	return buffer;
}

ID3D11SamplerState* DX11::CreateSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE mode)
{
	D3D11_SAMPLER_DESC samplerDescription;
	ZeroMemory(&samplerDescription, sizeof(D3D11_SAMPLER_DESC));
	samplerDescription.Filter = filter;
	samplerDescription.AddressU = mode;
	samplerDescription.AddressV = mode;
	samplerDescription.AddressW = mode;
	samplerDescription.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDescription.BorderColor[0] = 0.0f;
	samplerDescription.BorderColor[1] = 0.0f;
	samplerDescription.BorderColor[2] = 0.0f;
	samplerDescription.BorderColor[3] = 0.0f;

	ID3D11SamplerState* samplerState;
	ZeroMemory(&samplerState, sizeof(ID3D11SamplerState));
	device->CreateSamplerState(&samplerDescription, &samplerState);
	return samplerState;
}
