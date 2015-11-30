#include <Windows.h>
#include "Scene.h"
#include "resource.h"
static TCHAR szWindowClass[] = L"win32app";//Nombre de la clase que representa la ventana
static TCHAR szTitle[] = L"Sea Waves Experiment"; // Titulo de la ventana
static int WidthSize = 800;
static int HeightSize = 600;

Graphics* g;
Scene* scene;
int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wex; //Estructura para crear la ventana
	wex.cbSize = sizeof(WNDCLASSEX);
	wex.style = CS_HREDRAW | CS_VREDRAW; //Redibuja el contenido cuando el tamaño cambie
	wex.lpfnWndProc = WndProc; // Función que administra los mensajes que se obtienen de la ventana
	wex.cbClsExtra = 0;
	wex.cbWndExtra = 0;
	wex.hInstance = hInstance; //Instancia de la aplicación
	wex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); // icono de la isla
	wex.hCursor = LoadCursor(NULL, IDC_ARROW);//Se carga el cursor por defecto
	wex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wex.lpszMenuName = NULL;
	wex.lpszClassName = szWindowClass;
	wex.hIconSm = NULL;

	if (!RegisterClassEx(&wex)) //Se registra la clase de la ventana
	{
		return 1;
	}
	RECT rec = {0, 0, WidthSize, HeightSize};
	AdjustWindowRect(&rec, WS_OVERLAPPEDWINDOW, FALSE);//Se obtiene el tamaño cliente de la ventana
	HWND hWnd = CreateWindow(szWindowClass, szTitle, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, rec.right - rec.left, rec.bottom - rec.top, NULL, NULL, hInstance, NULL); // Se crea la ventana
	if (!hWnd) //hWnd es una variable de tipo HANDLER que representa la ventana creada
	{
		return 1;
	}
	ShowWindow(hWnd, nCmdShow);//Se muestra la ventana
	g = new Graphics(hWnd, rec.right - rec.left, rec.bottom - rec.top);//Se inicializa los componentes gráficos principales
	if (!g->Initialize())
		return 1;
	scene = new Scene(g);
	if (!scene->Initialize())// Se inicializa la escena
	{
		delete scene;
		return 1;
	}
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (WM_QUIT != msg.message)//Ciclo de vida de la aplicación, mientras no se obtenga un mensaje de salida de la aplicación sigue corriendo
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) //Or use an if statement
		{
			TranslateMessage(&msg); //Procesa los mensajes del teclado
			DispatchMessage(&msg);//Procesa los mensaje a la función WndProc
		}
		scene->Render();//Se pinta la escena
	}
	delete scene;
	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SIZE:
		g->SizeEvent(); //Gestiona el cambio de tamaño de la ventana
		break;

	case WM_SYSCOMMAND:
		if (lParam == VK_RETURN) //Entra si se presiona Alt + Enter
		{
			g->SwitchMode();//Administra el modo pantalla completa y el modo ventana
		}
		else 
		{
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);//Ejecuta la función para cerrar la ventana.
		break;
	default: return DefWindowProc(hWnd, Msg, wParam, lParam);//Para los demás mensajes ejecuta su comportamiento por defecto.
		break;
	}
	return 0;
}
