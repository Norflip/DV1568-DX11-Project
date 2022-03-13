#pragma once
#include <vector>
#include "Texture.h"
#include "Model.h"
#include "Mesh.h"
#include "OBJLoader.h"

namespace TerrainGenerator
{
	inline Mesh CreateTerrainPlane(size_t width, size_t height, ID3D11Device* device)
	{
		std::vector<VERTEX> vertexes;// (texture.width * texture.height);
		std::vector<unsigned int> indices;// (texture.width * texture.height * 6);

		int vi = 0;
		int ti = 0;
		const int size = 64;

		for (int y = 0; y < size + 1; y++)
		{
			for (int x = 0; x < size + 1; x++)
			{
				VERTEX vertex;
				vertex.position = { (float)x, 0, (float)y };

				float u = (float)x / size;
				float v = (float)y / size;

				vertex.uv = {u, v};
				vertex.normal = { 0,1,0 };
				vertexes.push_back(vertex);

				if (x < size && y < size)
				{
					indices.push_back(vi + size + 1);
					indices.push_back(vi + size + 2);
					indices.push_back(vi + 1);

					indices.push_back(vi + size + 1);
					indices.push_back(vi + 1);
					indices.push_back(vi);
				}

				vi++;
			}
		}

		return OBJLoader::CreateMesh(vertexes, indices, device);
	}

	inline std::vector<Model> CreateChunks(size_t cols, size_t rows, std::string heightMapPath, Shader* shader, DX11* dx11)
	{
		Material* material = new Material(shader);

		Texture t (heightMapPath, dx11->device);
		material->AddTexture(t.srv, TextureType::Vertex);
		material->AddSampler(dx11->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP), TextureType::Vertex);

		std::cout << t.width << " x " << t.height << std::endl;

		Mesh plane = CreateTerrainPlane(64, 64, dx11->device);
		std::vector<Model> chunks;

		for (size_t y = 0; y < rows; y++)
		{
			for (size_t x = 0; x < cols; x++)
			{
				Model md(plane, material);
				md.position = { 
					(float)x * (t.width), 
					0.0f, 
					(float)y * (t.height) 
				};
				chunks.push_back(md);
			}
		}

		return chunks;
	}
}