#define STB_IMAGE_IMPLEMENTATION   // use of stb functions once and for all
#include "stb_image.h"
#include "Texture.h"

Texture::Texture() : srv(nullptr) {}

Texture::Texture(std::string path, ID3D11Device* device) : srv(nullptr), buffer(nullptr)
{
	Load(path, device);
}

Texture::~Texture()
{
}

float Texture::GetHeight(int x, int y, int channels) const
{
	int pixelIndex = (width * y + x) * channels;
	return (static_cast<float>(buffer[pixelIndex]) / 255.0f);
}

void Texture::Load(std::string path, ID3D11Device* device)
{
	buffer = stbi_load(path.c_str(), &width, &height, &channels, 4);
	assert(buffer);

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureResourceData;
	ZeroMemory(&textureResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	textureResourceData.pSysMem = buffer;
	textureResourceData.SysMemPitch = width * 4;

	ID3D11Texture2D* texture;
	ZeroMemory(&texture, sizeof(ID3D11Texture2D));
	device->CreateTexture2D(&textureDesc, &textureResourceData, &texture);

	ZeroMemory(&srv, sizeof(ID3D11ShaderResourceView));
	HRESULT createtex = device->CreateShaderResourceView(texture, nullptr, &srv);
	assert(SUCCEEDED(createtex));

	texture->Release();
	texture = 0;



}
