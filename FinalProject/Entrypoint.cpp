#include <Windows.h>
#include "Scene.h"
#include "resource.h"
static TCHAR szWindowClass[] = L"win32app";
static TCHAR szTitle[] = L"Sea Waves Experiment";
static int WidthSize = 800;
static int HeightSize = 600;

Graphics* g;
Scene* scene;
int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wex;
	wex.cbSize = sizeof(WNDCLASSEX);
	wex.style = CS_HREDRAW | CS_VREDRAW;
	wex.lpfnWndProc = WndProc;
	wex.cbClsExtra = 0;
	wex.cbWndExtra = 0;
	wex.hInstance = hInstance;
	wex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wex.lpszMenuName = NULL;
	wex.lpszClassName = szWindowClass;
	wex.hIconSm = NULL;

	if (!RegisterClassEx(&wex))
	{
		return 1;
	}
	RECT rec = {0, 0, WidthSize, HeightSize};
	AdjustWindowRect(&rec, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindow(szWindowClass, szTitle, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, rec.right - rec.left, rec.bottom - rec.top, NULL, NULL, hInstance, NULL);
	if (!hWnd) 
	{
		return 1;
	}
	ShowWindow(hWnd, nCmdShow);
	g = new Graphics(hWnd, rec.right - rec.left, rec.bottom - rec.top);
	if (!g->Initialize())
		return 1;
	scene = new Scene(g);
	if (!scene->Initialize())
	{
		g->Release();
		delete g;
		delete scene;
		return 1;
	}
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (WM_QUIT != msg.message)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) //Or use an if statement
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		scene->Render();
	}
	delete scene;
	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SYSCOMMAND:
		if (lParam == VK_RETURN)
		{
			g->SwitchMode();
		}
		else 
		{
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, Msg, wParam, lParam);
		break;
	}

	return 0;
}