#include "Graphics.h"


Graphics::Graphics(HWND hWnd, int Width, int Height)
{
	this->hWnd = hWnd;
	this->Width = Width;
	this->Height = Height;
}


bool Graphics::Initialize()  
{
	if (FAILED(CreateDeviceAndSwapChain()))
		return false;
	if (FAILED(SetRenderTargets()))
		return false;
	SetViewPort();
	return true;
}

HRESULT Graphics::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = this->Width;
	sd.BufferDesc.Height = this->Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	//CSAA 16xQ (Quality)
	sd.SampleDesc.Count = 8;
	sd.SampleDesc.Quality = 16;
	sd.Windowed = TRUE;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0};
	D3D_FEATURE_LEVEL outLevel;
	HRESULT hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
#if _DEBUG
		D3D11_CREATE_DEVICE_DEBUG,
#else
		0,
#endif
		FeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&pDevice,
		&outLevel,
		&pDevContext);
	CComPtr<IDXGIDevice> pDXGIDevice;
	hr = pDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));
	if (FAILED(hr)) { return hr; }
	CComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	if (FAILED(hr)) { return hr; }
	CComPtr<IDXGIFactory> pIDXGIFactory;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));
	if (FAILED(hr)) { return hr; }
	hr = pIDXGIFactory->CreateSwapChain(pDevice, &sd, &pSwapChain);
	pIDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES);
	return hr;
}

HRESULT Graphics::SetRenderTargets()
{
	CComPtr<ID3D11Texture2D> pBackBuffer, DepthBuffer;
	HRESULT hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(hr))
		return hr;
	D3D11_TEXTURE2D_DESC ds; 
	ZeroMemory(&ds, sizeof(D3D11_TEXTURE2D_DESC));
	ds.MipLevels = 1;
	ds.ArraySize = 1;
	ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ds.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ds.Height = this->Height;
	ds.Width = this->Width;
	ds.SampleDesc.Count = 8;
	ds.SampleDesc.Quality = 16;
	ds.Usage = D3D11_USAGE_DEFAULT;

	hr = pDevice->CreateTexture2D(&ds, NULL, &DepthBuffer);
	if (FAILED(hr))
		return hr;
	hr = pDevice->CreateDepthStencilView(DepthBuffer, NULL, &pDepthStencilView);
	if (FAILED(hr))
		return hr;
	hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	if (FAILED(hr))
		return hr;
	ID3D11RenderTargetView* rtv[1] = { pRenderTargetView };
	pDevContext->OMSetRenderTargets(1, rtv, pDepthStencilView);
	return hr;
}

void Graphics::SetViewPort()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)this->Width;
	vp.Height = (float)this->Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pDevContext->RSSetViewports(1, &vp);
	IsReady = false;
}

HRESULT Graphics::SwitchMode()
{
	IsReady = false;
	BOOL IsFullScreenMode;
	HRESULT hr = pSwapChain->GetFullscreenState(&IsFullScreenMode, NULL);
	if (FAILED(hr))
		return hr;
	pDevContext->OMSetRenderTargets(0, 0, 0);
	pRenderTargetView.Release();
	pDepthStencilView.Release();
	int BufferCount = 1;
	if (IsFullScreenMode)
		hr = pSwapChain->SetFullscreenState(FALSE, NULL);
	else
		hr = pSwapChain->SetFullscreenState(TRUE, NULL);
	if (FAILED(hr))
		return false;
	if (IsFullScreenMode) 
	{
		RECT screen = { 0, 0, 800, 600 };
		AdjustWindowRect(&screen, WS_OVERLAPPEDWINDOW, FALSE);
		Width = screen.right - screen.left;
		Height = screen.bottom - screen.top;
	}
	else 
	{
		Width = 1440;
		Height = 900;
		BufferCount = 3;
	}
	DXGI_MODE_DESC ds;
	ds.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ds.RefreshRate.Numerator = 1;
	ds.RefreshRate.Denominator = 1;
	ds.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	ds.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	ds.Width = Width;
	ds.Height = Height;
	hr = pSwapChain->ResizeTarget(&ds);
	if (FAILED(hr))
		return false;
	hr = pSwapChain->ResizeBuffers(BufferCount, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
	SetViewPort();
	return hr;
}

void Graphics::Release() 
{
	pSwapChain->SetFullscreenState(FALSE, NULL);
}

void Graphics::Ready(bool IsReady)
{
	this->IsReady = IsReady;
}

Graphics::~Graphics()
{
}
