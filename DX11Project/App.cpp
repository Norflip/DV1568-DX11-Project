#include "App.h"

App::App(HWND hwnd, size_t width, size_t height) : dx11(), hwnd(hwnd), width(width), height(height), terrain(nullptr)
{

}

void App::Initalize()
{
	srand(time(0));

	dx11.Initialize(hwnd, width, height);
	renderer.Initialize(&dx11, width, height);

	particleSystem.Initialize(&dx11);
	particleSystem.ignorePicking = true;
	particleSystem.SetEmitterPosition({ 0, 12, 0 });

	dynamicCube.Initialize(&dx11);
	dynamicCube.ignorePicking = true;

	camera = new Camera(60.0f, width, height);
	updateCamera = true;

	SetupObjects();
	start = ch::high_resolution_clock::now();
}

void App::Cleanup()
{
	dx11.Cleanup();
	renderer.Cleanup();

	// delete objects
}

void App::Update()
{
	timePassed = ch::duration<double, std::micro>(ch::high_resolution_clock::now() - start);
	double now = timePassed.count() / 1000.0;
	const float dt = (float)(now - prev) / 1000.0f;
	prev = now;

	for (size_t i = 0; i < 256; i++)
		keystates[i] = false;

	for (auto i : inputs)
		keystates[i.key] = i.state;

	UpdateCamera(dt);
	Pick();

	particleSystem.UpdatePositions(dt);
	dynamicCube.enabled = false;

	renderer.StartFrame();

	for (size_t i = 0; i < 6; i++)
	{
		//set rendertarget to gbuffer
		renderer.SetRenderTarget(dynamicCube.gBuffer);
		renderer.UnbindTextures(dynamicCube.gBuffer);

		//draw all objects
		renderer.DrawObjects(objects, dynamicCube.GetCamera(i));

		//set rendertarge to camera RTV [i]
		renderer.SetRenderTarget(dynamicCube.GetRenderTarget(i));
		renderer.BindTextures(dynamicCube.gBuffer);
	
		// draw quad
		renderer.DrawQuad();
	}

	//dx11.context->GenerateMips(dynamicCube.srv);
	dynamicCube.enabled = true;
	renderer.UnbindTextures(dynamicCube.gBuffer);
	
	renderer.SetGBufferRenderTarget();
	renderer.DrawObjects(objects, camera);
	renderer.DisplayFrame();

	this->leftMouseDownLastFrame = leftMouseDown;
}

void App::UpdateCamera(const float& dt)
{
	// ROTATE
	dx::XMFLOAT3 rotationDelta(cameraDelta.y * dt, cameraDelta.x * dt, 0.0f);
	camera->rotation = dx::XMVectorAdd(camera->rotation, dx::XMLoadFloat3(&rotationDelta));
	cameraDelta = { 0,0 };

	dx::XMFLOAT2 tranlateDirection = { 0,0 };
	if (keystates['w'])
		tranlateDirection.y += 1.0f;

	if (keystates['s'])
		tranlateDirection.y -= 1.0f;

	if (keystates['a'])
		tranlateDirection.x -= 1.0f;

	if (keystates['d'])
		tranlateDirection.x += 1.0f;

	if (keystates['e'])
		this->headHeight += 1.0f * dt;

	if (keystates['q'])
		this->headHeight -= 1.0f *  dt;


	float len = sqrtf(tranlateDirection.y * tranlateDirection.y + tranlateDirection.x * tranlateDirection.x);

	if (len != 0.0f)
	{
		dx::XMMATRIX rot = dx::XMMatrixRotationRollPitchYawFromVector(camera->rotation);

		dx::XMVECTOR right = dx::XMVector3Transform({ 1,0,0 }, rot);
		right = dx::XMVectorScale(right, tranlateDirection.x / len);

		dx::XMVECTOR forward = dx::XMVector3Transform({ 0,0,1 }, rot);
		forward = dx::XMVectorScale(forward, tranlateDirection.y / len);

		dx::XMVECTOR translation = dx::XMVectorAdd(forward, right);
		translation = dx::XMVectorScale(translation, FPS_CAMERA_SPEED * dt);
		camera->position = dx::XMVectorAdd(camera->position, translation);
	}

	if (terrain != nullptr)
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMStoreFloat3(&pos, camera->position);

		float y = terrain->SampleHeight(pos.x, pos.z) + headHeight;
		camera->position = DirectX::XMVectorSetByIndex(camera->position, y, 1);
	}

	camera->UpdateViewMatrix();
}

void App::Pick()
{
	if (this->leftMouseDown && !this->leftMouseDownLastFrame)
	{
		Ray ray = camera->ScreenToRay(mousePosition);
		int index = 0;

		for (auto i : objects)
		{
			RayHit hit = ray.IntersectsModel(i);
			
			if (hit.intersected)
			{
				const int accuracy = 1000;
				float r = (float)(rand() % accuracy) / (float)accuracy;
				float g = (float)(rand() % accuracy) / (float)accuracy;
				float b = 1.0f - r - g;

				i->GetMaterial()->properties.diffuse = { r,g,b,1.0 };
			}

			index++;
		}
	}
}

void App::SetKeyDown(char key, bool state)
{
	inputs.push_back({ key, state });
}

void App::SetupObjects()
{
	Shader* standard = new Shader();
	standard->SetVSPath(L"Standard_vs.hlsl");
	standard->SetGSPath(L"BackfaceCulling_gs.hlsl");
	standard->SetPSPath(L"Standard_ps.hlsl");
	standard->Load(dx11.device);

	Shader* terrainShader = new Shader();
	terrainShader->SetVSPath(L"Terrain_vs.hlsl");
	terrainShader->SetGSPath(L"BackfaceCulling_gs.hlsl");
	terrainShader->SetPSPath(L"Standard_ps.hlsl");
	terrainShader->Load(dx11.device);


	Model* monkeyRed = OBJLoader::LoadOBJ("Models/R_monkey.obj", standard, &dx11);
	monkeyRed->position = { 20 , 5, 20 };
	objects.push_back(monkeyRed);

	Model* monkeyBlue = OBJLoader::LoadOBJ("Models/B_monkey.obj", standard, &dx11);
	monkeyBlue->position = { 24 , 5, 20 };
	objects.push_back(monkeyBlue);

	Model* monkeyGreen = OBJLoader::LoadOBJ("Models/G_monkey.obj", standard, &dx11);
	monkeyGreen->position = { 28 , 5, 20 };
	objects.push_back(monkeyGreen);

	Model* wall = OBJLoader::LoadOBJ("Models/cube.obj", standard, &dx11);
	Material* mat = wall->GetMaterial();
	mat->AddTexture(Texture("Textures/Leather_001_COLOR.png", dx11.device).srv, TextureType::Pixel);
	mat->AddTexture(Texture("Textures/Leather_001_NORM.png", dx11.device).srv, TextureType::Pixel);
	mat->AddSampler(dx11.linearSampler, TextureType::Pixel);
	mat->properties.hasAlbedoTexture = true;
	mat->properties.hasNormalTexture = true;

	wall->position = { 30.0f , 2.5f, 42.0f };
	objects.push_back(wall);

	// MONEKYS AND CUBE
	dynamicCube.position = { 30.0f, 1.5f, 30.0f };
	dynamicCube.UpdateCameraPosition();

	for (int z = -1; z <= 1; z++)
	{
		for (int x = -1; x <= 1; x++)
		{
			if (x == 0 && z == 0)
				continue;

			Model* monkeyRed = OBJLoader::LoadOBJ("Models/R_monkey.obj", standard, &dx11);
			monkeyRed->position = { 30.0f + static_cast<float>(x * 3.5f) , 1.5f,  30.0f + static_cast<float>(z * 3.5f) };
			objects.push_back(monkeyRed);
		}
	}

	// TERRAIN
	this->terrain = new Terrain(terrainShader, &dx11);
	this->terrain->Initialize("Textures/height.png", &dx11);
	camera->SetTerrain(this->terrain);


	Model* pointLightModel = OBJLoader::LoadOBJ("Models/point.obj", standard, &dx11);
	std::vector<dx::XMFLOAT3> positions;

	for (size_t i = 0; i < 16; i++)
	{
		const int accuracy = 1000;
		float r = (float)(rand() % accuracy) / (float)accuracy;
		float g = (float)(rand() % accuracy) / (float)accuracy;
		float b = 1.0f - r - g;

		float x = (float)(rand() % accuracy) / (float)accuracy;
		x = 10.0f + x * 50.0f;

		float z = (float)(rand() % accuracy) / (float)accuracy;
		z = 10.0f + z * 50.0f;

		float y = terrain->SampleHeight(x, z);

		positions.push_back({ x, y + 4.0f ,z });

		Model* m = new Model(pointLightModel->GetMesh(), pointLightModel->GetMaterial());
		m->position = { x, y + 4.0f ,z };
		objects.push_back(m);
	}

	renderer.SetupLights(positions);
	delete pointLightModel;

	objects.push_back(this->terrain);
	objects.push_back(&particleSystem);
	objects.push_back(&dynamicCube);
}
