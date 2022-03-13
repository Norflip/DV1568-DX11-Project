#pragma once
#include <vector>
#include "DX11.h"
#include "Model.h"
#include "Camera.h"
#include "OBJLoader.h"
#include "ConstantBuffers.h"

class Renderer
{
public:
	const FLOAT CLEAR_COLOR[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
	const size_t GBUFFER_COUNT = 3;

public:
	Renderer();
	void Initialize(DX11* dx11, size_t width, size_t height);

	void StartFrame();

	void DrawObjects(std::vector<Model*>& objects, Camera* camera);
	void DisplayFrame();
	void Cleanup();

	void SetGBufferRenderTarget();
	void SetRenderTarget(RenderTarget target);

	void DrawQuad();
	void BindLightShader();
	
	void BindTextures(RenderTarget rt);
	void UnbindTextures(RenderTarget rt);

	void SetAndClearTarget(RenderTarget rt);
	
	void SetupLights(std::vector<dx::XMFLOAT3> pointLightPositions);

private:
	DX11* dx11;
	ID3D11Buffer* worldCBuffer_ptr;
	WorldCBuffer worldCBufferData;

	ID3D11Buffer* lightCBuffer_ptr;
	LightCBuffer lightCBufferData;
	
	Mesh screenquad;
	Shader* lightpass;
	RenderTarget gBuffer;
	RenderTarget backBuffer;

};