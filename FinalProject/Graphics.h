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
	void SetViewPort();
	void SizeEvent();
	~Graphics();
private:

	HRESULT CreateDeviceAndSwapChain();
	HRESULT SetRenderTargets();
	HWND hWnd;
	bool IsReady = false;
public:
	int BufferCount = 1;
	int Width, Height;
	static const float color[4]; 
	D3D11_VIEWPORT vp;
	CComPtr<IDXGISwapChain> pSwapChain;
	CComPtr<ID3D11Device> pDevice;
	CComPtr<ID3D11DeviceContext> pDevContext;

	CComPtr<ID3D11RenderTargetView> pRenderTargetView;
	CComPtr<ID3D11DepthStencilView> pDepthStencilView;
};

