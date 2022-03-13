#pragma once
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "OBJLoader.h"
#include <cmath>

class Terrain : public Model
{
	const float TERRAIN_SCALE = 5.0f;

public:
	Terrain(Shader* shader, DX11* dx11);
	virtual ~Terrain();

	void Initialize(std::string heightMap, DX11* dx11);
	float SampleHeight(float x, float z) const;

private:
	float Lerp(float a, float b, float t) const;

private:
	std::vector<VERTEX> vertexes;
	std::vector<unsigned int> indices;

	Texture heightMap;
	const int SIZE = 64;

};