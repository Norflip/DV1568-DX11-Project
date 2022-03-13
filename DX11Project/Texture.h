#pragma once
#include <string>
#include "DX11.h"

class Texture
{
public:
	Texture();
	Texture(std::string path, ID3D11Device* device);
	virtual ~Texture();

	float GetHeight(int x, int y, int channels) const;

	ID3D11ShaderResourceView* srv;
	int width, height, channels;
	unsigned char* buffer;

private:
	void Load(std::string path, ID3D11Device* device);
};