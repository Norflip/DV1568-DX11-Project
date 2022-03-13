#include "Material.h"

Material::Material(Shader* shader, DX11* dx11) : shader(shader), dx11(dx11)
{
	properties.ambient = { 1,1,1,1 }; //ambient color
	properties.diffuse = { 1,1,1,1 }; // diffuse color
	properties.hasAlbedoTexture = false;	//map_Ka
	properties.hasNormalTexture = false;	//map_bump

	buffer = dx11->CreateConstantBuffer((void*)&properties, sizeof(properties));
	AddSampler(dx11->linearSampler, TextureType::Pixel);
}

Material::~Material()
{
}

void Material::AddTexture(ID3D11ShaderResourceView* texture, TextureType type)
{
	textures[(int)type].push_back(texture);
}

void Material::AddSampler(ID3D11SamplerState* sampler, TextureType type)
{
	samplers[(int)type].push_back(sampler);
}

void Material::Bind()
{
	shader->Bind(dx11->context);

	// BIND BUFFER
	dx11->context->UpdateSubresource(buffer, 0, 0, &properties, 0, 0);
	dx11->context->PSSetConstantBuffers(MATERIAL_BUFFER_SLOT, 1, &buffer);

	for (size_t j = 0; j < 3; j++)
	{
		TextureType type = (TextureType)j;

		for (size_t i = 0; i < textures[j].size(); i++)
		{
			switch (type)
			{
			case TextureType::Pixel: dx11->context->PSSetShaderResources(i, 1, &textures[j][i]); break;
			case TextureType::Vertex: dx11->context->VSSetShaderResources(i, 1, &textures[j][i]); break;
			case TextureType::Geometry: dx11->context->GSSetShaderResources(i, 1, &textures[j][i]); break;
			}
		}

		for (size_t i = 0; i < samplers[j].size(); i++)
		{
			switch (type)
			{
			case TextureType::Pixel: dx11->context->PSSetSamplers(i, 1, &samplers[j][i]); break;
			case TextureType::Vertex: dx11->context->VSSetSamplers(i, 1, &samplers[j][i]); break;
			case TextureType::Geometry: dx11->context->GSSetSamplers(i, 1, &samplers[j][i]); break;
			}
		}
	}
}

void Material::Unbind()
{
	shader->Bind(dx11->context);

	for (size_t j = 0; j < 3; j++)
	{
		TextureType type = (TextureType)j;

		ID3D11ShaderResourceView* const nullsrv[1] = { NULL };
		ID3D11SamplerState* const nullsampler[1] = { NULL };

		for (size_t i = 0; i < textures[j].size(); i++)
		{
			switch (type)
			{
			case TextureType::Pixel: dx11->context->PSSetShaderResources(i, 1, nullsrv); break;
			case TextureType::Vertex: dx11->context->VSSetShaderResources(i, 1, nullsrv); break;
			case TextureType::Geometry: dx11->context->GSSetShaderResources(i, 1, nullsrv); break;
			}
		}

		for (size_t i = 0; i < samplers[j].size(); i++)
		{
			switch (type)
			{
			case TextureType::Pixel: dx11->context->PSSetSamplers(i, 1, nullsampler); break;
			case TextureType::Vertex: dx11->context->VSSetSamplers(i, 1, nullsampler); break;
			case TextureType::Geometry: dx11->context->GSSetSamplers(i, 1, nullsampler); break;
			}
		}
	}
}

