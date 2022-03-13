#pragma once
#include <d3d11.h>
#include <dxgi.h>     // används för swapchain och device / deviceContext
#include <DirectXMath.h>
#include <assert.h>

#pragma comment(lib, "gdi32")
#pragma comment(lib, "d3d11") 
#pragma comment( lib, "dxgi" )   
#pragma comment(lib, "d3dcompiler.lib")


struct RenderTarget
{
	size_t count;
	ID3D11RenderTargetView** rtv;
	ID3D11ShaderResourceView** srv;
	ID3D11DepthStencilView* dsv;
	D3D11_VIEWPORT viewport;
};

class DX11
{
public:
	DX11();
	void Initialize(HWND& hwnd, size_t width, size_t height);
	void Cleanup();

	RenderTarget CreateBackBuffer(size_t width, size_t height) const;
	RenderTarget CreateRenderTargets(size_t count, size_t width, size_t height) const;
	void CreateDepthStencilView(RenderTarget& rt, size_t width, size_t height) const;

	ID3D11Buffer* CreateConstantBuffer(void* data, size_t size);
	ID3D11SamplerState* CreateSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE mode);

	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapchain;
	ID3D11SamplerState* linearSampler;

private:
	ID3D11RasterizerState* rasterizerState;
};