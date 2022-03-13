#pragma once
#include <assert.h>
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>

#include "DX11.h"
#include "Texture.h"
#include "Terrain.h"
#include "Model.h"
#include "Renderer.h"
#include "ParticleSystem.h"
#include "DynamicCube.h"

namespace ch = std::chrono;
namespace dx = DirectX;

class App
{
	struct KeyInput
	{
		char key;
		bool state;
	};

	const float FPS_CAMERA_SPEED = 10.0f;

public:
	App(HWND hwnd, size_t width, size_t height);
	void Initalize();
	void Cleanup();
	void Update();

	void SetKeyDown(char key, bool state);
	void SetLeftMouseButtonState(bool state) { this->leftMouseDown = state; }
	void SetMouseDelta(dx::XMFLOAT2 delta) { this->cameraDelta = delta; }
	void SetMousePosition(dx::XMFLOAT2 mousePosition) { this->mousePosition = mousePosition; }

private:
	void UpdateCamera(const float& dt);
	void Pick();
	void SetupObjects();

private:
	size_t width, height;
	HWND hwnd;
	DX11 dx11;
	Renderer renderer;

	float headHeight = 1.7f;

	// INPUT
	bool leftMouseDown;
	bool leftMouseDownLastFrame;
	bool keystates[256];
	std::vector<KeyInput> inputs;
	dx::XMFLOAT2 cameraDelta;
	dx::XMFLOAT2 mousePosition;

	Camera* camera;
	Terrain* terrain;
	ParticleSystem particleSystem;
	DynamicCube dynamicCube;

	bool updateCamera = true;
	std::vector<Model*> objects;


	// TIME 
	ch::time_point<ch::steady_clock> start;
	ch::duration<double, std::micro> timePassed;
	double prev = 0.0;
};
