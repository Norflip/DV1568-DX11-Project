#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#include <assert.h>
#include "App.h"


HWND OpenWindow(HINSTANCE hInstance, size_t width, size_t height);
void OpenConsole();

static LRESULT CALLBACK HandleWindowProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL ContainsPoint(const POINTS& pt);

constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;
App* app;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	OpenConsole();
	HWND hwnd = OpenWindow(hInstance, WINDOW_WIDTH, WINDOW_HEIGHT);

	app = new App(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);
	app->Initalize();

	MSG message;
	ZeroMemory(&message, sizeof(MSG));

	while (TRUE)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);

			if (message.message == WM_QUIT)
				break;
		}
		else
		{
			app->Update();
		}
	}

	app->Cleanup();
	return 0;
}

HWND OpenWindow(HINSTANCE hInstance, size_t width, size_t height)
{
	const auto projectTitel = L"dx11project";

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = HandleWindowProc;
	wc.lpszClassName = projectTitel;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	RegisterClass(&wc);

	RECT rect;
	rect.top = rect.left = 400;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	AdjustWindowRect(&rect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	HWND hwnd = CreateWindowExW(0, projectTitel, projectTitel, WS_OVERLAPPEDWINDOW, rect.top, rect.left, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);
	assert(hwnd);
	ShowWindow(hwnd, SW_SHOW);

	//MOUSE & KEYBOARD
	RAWINPUTDEVICE rid[2];
	ZeroMemory(&rid, sizeof(rid));

	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = 0;
	rid[0].hwndTarget = hwnd;

	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;
	rid[1].dwFlags = 0;
	rid[1].hwndTarget = hwnd;

	assert(RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)));

	// center cursor
	RECT mousearea;
	ZeroMemory(&mousearea, sizeof(RECT));
	mousearea.bottom = height;
	mousearea.right = width;
	mousearea.top = mousearea.bottom = height / 2;
	mousearea.left = mousearea.right = width / 2;

	MapWindowPoints(hwnd, nullptr, reinterpret_cast<POINT*>(&mousearea), 2);
	ClipCursor(&mousearea);
	return hwnd;
}

void OpenConsole()
{
	AllocConsole();
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hConsole = _open_osfhandle((long)stdHandle, _O_TEXT);
	FILE* fp = _fdopen(hConsole, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);
}

LRESULT HandleWindowProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	switch (umsg)
	{
	case WM_MOUSEMOVE:
	{
		POINTS pt = MAKEPOINTS(lParam);

		if (ContainsPoint(pt))
			app->SetMousePosition({ static_cast<float>(pt.x), static_cast<float>(pt.y) });
		
		break;
	}

	case WM_LBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		app->SetLeftMouseButtonState(true);
		break;
	}

	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS(lParam);
		app->SetLeftMouseButtonState(false);
		break;
	}

	// https://docs.microsoft.com/sv-se/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement?redirectedfrom=MSDN
	case WM_INPUT:
	{
		UINT size{};
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
			break;

		LPBYTE buffer = new BYTE[size];
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
			break;

		RAWINPUT* raw = (RAWINPUT*)buffer;
		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			app->SetMouseDelta({ static_cast<float>(raw->data.mouse.lLastX), static_cast<float>(raw->data.mouse.lLastY) });
		}
		else if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			char key = (char)tolower((char)raw->data.keyboard.VKey);
			app->SetKeyDown(key, !(raw->data.keyboard.Flags & RI_KEY_BREAK));
		}
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, umsg, wParam, lParam);
}

BOOL ContainsPoint(const POINTS& pt)
{
	return pt.x >= 0 && pt.x < (SHORT)WINDOW_WIDTH&& pt.y >= 0 && pt.y < (SHORT)WINDOW_HEIGHT;
}
