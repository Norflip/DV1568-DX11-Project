#pragma once
#include "Model.h"
#include "Camera.h"

class DynamicCube : public Model
{
	const size_t DYNAMIC_RESOLUTION = 256;

public:
	DynamicCube();
	virtual ~DynamicCube();

	void Initialize(DX11* dx11);
	void UpdateCameraPosition();

	RenderTarget GetRenderTarget(size_t i);
	Camera* GetCamera(size_t i);

	RenderTarget gBuffer;
	ID3D11ShaderResourceView* srv;

private:
	void SetupCameras();
	
private:
	DX11* dx11;
	Camera* cameras[6];
	ID3D11RenderTargetView* rtv[6];
	ID3D11DepthStencilView* dsv;

};