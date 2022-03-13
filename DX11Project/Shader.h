#pragma once
#include <d3d11.h>    // D3D 11
#include <dxgi.h>     // används för swapchain och device / deviceContext
#include <d3dcompiler.h> // for shaders
#include "DX11.h"

class Shader
{
public:
	Shader();
	virtual ~Shader();

	void SetPSPath(LPCWSTR ps) { this->ps = ps; }
	void SetVSPath(LPCWSTR vs) { this->vs = vs; }
	void SetGSPath(LPCWSTR gs) { this->gs = gs; }
	void Load(ID3D11Device* device, bool isParticle = false);
	void Bind(ID3D11DeviceContext* context);


private:

	void LoadPS(ID3D11Device* device);
	void LoadGS(ID3D11Device* device);
	void LoadVS(ID3D11Device* device, bool isParticle);

	DWORD flags;
	ID3D11PixelShader* pixelShader;
	ID3D11GeometryShader* geometryShader;
	ID3D11VertexShader* vertexShader;
	ID3D11InputLayout* inputLayout;

	LPCWSTR vs, ps, gs;
};