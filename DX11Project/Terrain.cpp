#include "Terrain.h"

Terrain::Terrain(Shader* shader, DX11* dx11) : Model()
{
	this->ignorePicking = true;
	SetMaterial(new Material(shader, dx11));
	GetMaterial()->properties.diffuse = { 0.1f, 0.8f, 0.2f, 1.0f };
}

Terrain::~Terrain()
{
}

void Terrain::Initialize(std::string heightMapPath, DX11* dx11)
{
	heightMap = Texture(heightMapPath, dx11->device);
	GetMaterial()->AddTexture(heightMap.srv, TextureType::Vertex);
	GetMaterial()->AddSampler(dx11->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP), TextureType::Vertex);

	int vi = 0;

	for (int z = 0; z < SIZE + 1; z++)
	{
		for (int x = 0; x < SIZE + 1; x++)
		{
			VERTEX vertex;
			vertex.position = { (float)x, 0.0f, (float)z };

			vertex.uv = { (float)x / SIZE, (float)z / SIZE };
			vertex.normal = { 0,1,0 };
			vertexes.push_back(vertex);

			if (x < SIZE && z < SIZE)
			{
				indices.push_back(vi + SIZE + 1);
				indices.push_back(vi + SIZE + 2);
				indices.push_back(vi + 1);

				indices.push_back(vi + SIZE + 1);
				indices.push_back(vi + 1);
				indices.push_back(vi);
			}

			vi++;
		}
	}

	SetMesh(OBJLoader::CreateMesh(vertexes, indices, dx11->device));
}

float Terrain::SampleHeight(float x, float z) const
{
	int col = (int)floorf(x);
	int row = (int)floorf(z);
	float height = 0.0f;

	if (row > 0 && col > 0 && row < SIZE && col < SIZE)
	{
		float bl = heightMap.GetHeight(col, row, 4);
		float br = heightMap.GetHeight(col + 1, row, 4);
		float tr = heightMap.GetHeight(col + 1, row + 1, 4);
		float tl = heightMap.GetHeight(col, row + 1, 4);
		
		float u = x - col; 
		float v = z - row;
		height = Lerp(Lerp(bl, br, u), Lerp(tl, tr, u), v);
	}

	return height * TERRAIN_SCALE;
}

float Terrain::Lerp(float a, float b, float t) const
{
	return a * (1.0f - t) + b * t;
}