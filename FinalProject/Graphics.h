#pragma once

#include <atlbase.h>
#include <d3d11.h>
//Se incluye la librería necesaria para hacer funcionar la API de Direct3D 11
#pragma comment(lib, "d3d11.lib")
//Con estos parámetros AA estará desactivado para que está aplicación corra en más computadores.
#define SAMPLE_COUNT 1
#define QUALITY_LEVEL 0

class Graphics
{
public:
	Graphics(HWND hWnd, int Width, int Height);
	//Se inicializan los componentes gráficos	
	bool Initialize();
	//Se administra el cambio de modo ventana a pantalla completa y viceversa
	HRESULT SwitchMode();
	//Esta función se ejecuta antes de eliminar este objeto
	void Release();
	void Ready(bool);
	inline bool isReady() { return IsReady; };
	void SetViewPort();
	void SizeEvent();
	~Graphics();
private:

	HRESULT CreateDeviceAndSwapChain();
	HRESULT SetRenderTargets();
	HWND hWnd;
	bool IsReady;
public:
	int BufferCount;
	int Width, Height;
	static const float color[4]; 
	//estructura que define el áre de pintado.
	D3D11_VIEWPORT vp;
	CComPtr<IDXGISwapChain> pSwapChain; //"Cadena de intercambio", gestiona la forma en que el contenido es pintado en pantalla.
	CComPtr<ID3D11Device> pDevice; //contiene funciones para inicializar recursos
	CComPtr<ID3D11DeviceContext> pDevContext; //Contiene funciones para configurar la graphics Pipeline y pintar los gráficos

	CComPtr<ID3D11RenderTargetView> pRenderTargetView;//Contiene el back Buffer
	CComPtr<ID3D11DepthStencilView> pDepthStencilView;//Contiene el Depth Buffer
};

