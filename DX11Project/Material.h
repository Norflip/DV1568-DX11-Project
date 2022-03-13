#pragma once
#include <Windows.h>
#include <d3d11.h>    // D3D 11
#include <dxgi.h>     // används för swapchain och device / deviceContext
#include <vector>

#include "DX11.h"
#include "Shader.h"
#include <iostream>
#include <string>
#include "ConstantBuffers.h"
#include <DirectXMath.h>
namespace dx = DirectX;

enum class TextureType
{
	Pixel = 0,
	Vertex = 1,
	Geometry = 2
};

class Material
{
public:
	Material(Shader* shader, DX11* dx11);
	virtual ~Material();

	void AddTexture(ID3D11ShaderResourceView* texture, TextureType type);
	void AddSampler(ID3D11SamplerState* sampler, TextureType type);
	void Bind();
	void Unbind();
	
	MaterialCBuffer properties;

private:
	Shader* shader;
	DX11* dx11;

	std::vector<ID3D11ShaderResourceView*> textures[3];
	std::vector<ID3D11SamplerState*> samplers[3];
	ID3D11Buffer* buffer;
};