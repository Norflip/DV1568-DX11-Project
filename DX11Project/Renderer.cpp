#include "Renderer.h"

Renderer::Renderer()
{
}

void Renderer::Initialize(DX11* dx11, size_t width, size_t height)
{
	this->dx11 = dx11;

	lightpass = new Shader();
	lightpass->SetPSPath(L"DeferredLights_ps.hlsl");
	lightpass->SetVSPath(L"DeferredLights_vs.hlsl");
	lightpass->Load(dx11->device);

	gBuffer = dx11->CreateRenderTargets(GBUFFER_COUNT, width, height);
	backBuffer = dx11->CreateBackBuffer(width, height);
	worldCBuffer_ptr = dx11->CreateConstantBuffer((void*)&worldCBufferData, sizeof(worldCBufferData));

	// SCREEN QUAD
	std::vector<VERTEX> vertices =
	{
		VERTEX({ -1.0f, -1.0f, 0 }, { 0,1 }, { 0,0,0 }, { 0,0,0 }),		// 0,0
		VERTEX({ 1.0f, -1.0f, 0 }, { 1,1 }, { 0,0,0 }, { 0,0,0 }),		// 0, w
		VERTEX({ 1.0f, 1.0f, 0 }, { 1,0 }, { 0,0,0 }, { 0,0,0 }),		// h, w
		VERTEX({ -1.0f, 1.0f, 0 }, { 0,0 }, { 0,0,0 }, { 0,0,0 })		// h, 0
	};

	screenquad = OBJLoader::CreateMesh(vertices, { 3,2,1, 3,1,0 }, dx11->device);
}

void Renderer::StartFrame()
{
	dx11->context->UpdateSubresource(lightCBuffer_ptr, 0, 0, &lightCBufferData, 0, 0);
	dx11->context->PSSetConstantBuffers(LIGHT_BUFFER_SLOT, 1, &lightCBuffer_ptr);
}

void Renderer::DrawObjects(std::vector<Model*>& objects, Camera* camera)
{
	for (size_t i = 0; i < objects.size(); i++)
	{
		if (objects[i]->enabled)
		{
			objects[i]->GetMaterial()->Bind();
			worldCBufferData.model = DirectX::XMMatrixTranspose(objects[i]->GetModelMatrix());
			worldCBufferData.view = DirectX::XMMatrixTranspose(camera->GetViewMatrix());
			worldCBufferData.projection = DirectX::XMMatrixTranspose(camera->GetProjectionMatrix());
			dx::XMStoreFloat3(&worldCBufferData.eye, camera->position);

			dx11->context->UpdateSubresource(worldCBuffer_ptr, 0, 0, &worldCBufferData, 0, 0);
			dx11->context->VSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, &worldCBuffer_ptr);
			dx11->context->GSSetConstantBuffers(WORLD_BUFFER_SLOT, 1, &worldCBuffer_ptr);


			dx::XMStoreFloat3(&lightCBufferData.eye, camera->position);
			dx11->context->UpdateSubresource(lightCBuffer_ptr, 0, 0, &lightCBufferData, 0, 0);
			dx11->context->PSSetConstantBuffers(LIGHT_BUFFER_SLOT, 1, &lightCBuffer_ptr);

			objects[i]->Draw(dx11->context);
			objects[i]->GetMaterial()->Unbind();
		}
	}
}

void Renderer::DisplayFrame()
{
	SetAndClearTarget(backBuffer);
	BindTextures(gBuffer);
	DrawQuad();

	dx11->swapchain->Present(1, 0);
}

void Renderer::Cleanup()
{
	//gbuffer
	//backbuffer
}


void Renderer::SetGBufferRenderTarget()
{
	SetRenderTarget(gBuffer);
}

void Renderer::SetRenderTarget(RenderTarget target)
{
	UnbindTextures(target);
	SetAndClearTarget(target);
}

void Renderer::SetAndClearTarget(RenderTarget rt)
{
	dx11->context->RSSetViewports(1, &rt.viewport);
	dx11->context->OMSetRenderTargets(rt.count, rt.rtv, rt.dsv);

	for (size_t i = 0; i < rt.count; i++)
		dx11->context->ClearRenderTargetView(rt.rtv[i], CLEAR_COLOR);

	if (rt.dsv != nullptr)
		dx11->context->ClearDepthStencilView(rt.dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::SetupLights(std::vector<dx::XMFLOAT3> pointLightPositions)
{
	dx::XMVECTOR sunDirection = { 1, -1, -1 };
	dx::XMStoreFloat3(&lightCBufferData.direction, dx::XMVector3Normalize(sunDirection));
	lightCBufferData.sunColor = { 0.945f, 0.933f, 0.698f, 1.0f };
	lightCBufferData.lightCount = pointLightPositions.size();
	lightCBufferData.sunStrength = 0.2f;

	for (size_t i = 0; i < pointLightPositions.size(); i++)
	{
		const int accuracy = 1000;
		float r = (float)(rand() % accuracy) / (float)accuracy;
		float g = (float)(rand() % accuracy) / (float)accuracy;
		float b = 1.0f - r - g;

		PointLight pl;
		pl.position = pointLightPositions[i];
		pl.color = { r,g,b, 1.0f};
		pl.radius = 10.0f;
		lightCBufferData.lights[i] = pl;
	}

	lightCBuffer_ptr = dx11->CreateConstantBuffer((void*)&lightCBufferData, sizeof(lightCBufferData));
}

void Renderer::DrawQuad()
{
	BindLightShader();
	dx11->context->PSSetSamplers(0, 1, &dx11->linearSampler);

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	dx11->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dx11->context->IASetVertexBuffers(0, 1, &screenquad.vertexBuffer, &stride, &offset);
	dx11->context->IASetIndexBuffer(screenquad.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	dx11->context->DrawIndexed(screenquad.indicies.size(), 0, 0);
}

void Renderer::BindLightShader()
{
	lightpass->Bind(dx11->context);
}

void Renderer::BindTextures(RenderTarget rt)
{
	for (size_t i = 0; i < rt.count; i++)
		dx11->context->PSSetShaderResources(i, 1, &rt.srv[i]);
}

void Renderer::UnbindTextures(RenderTarget rt)
{
	for (size_t i = 0; i < rt.count; i++)
	{
		ID3D11ShaderResourceView* nullSRV[1];
		ZeroMemory(&nullSRV, sizeof(nullSRV));
		dx11->context->PSSetShaderResources(i, 1, nullSRV);
	}
}
