#pragma once

#include <atlbase.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

class Graphics
{
public:
	Graphics(HWND hWnd, int Width, int Height);
	bool Initialize();
	HRESULT SwitchMode();
	void Release();
	void Ready(bool);
	inline bool isReady() { return IsReady; };
	~Graphics();
private:

	HRESULT CreateDeviceAndSwapChain();
	HRESULT SetRenderTargets();
	void SetViewPort();
	HWND hWnd;
	bool IsReady = false;
public:
	int Width, Height;

	CComPtr<IDXGISwapChain> pSwapChain;
	CComPtr<ID3D11Device> pDevice;
	CComPtr<ID3D11DeviceContext> pDevContext;

	CComPtr<ID3D11RenderTargetView> pRenderTargetView;
	CComPtr<ID3D11DepthStencilView> pDepthStencilView;
};

