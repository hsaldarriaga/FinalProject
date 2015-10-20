#include "Graphics.h"


Graphics::Graphics(HWND hWnd, int Width, int Height)
{
	this->hWnd = hWnd;
	this->Width = Width;
	this->Height = Height;
}

const float Graphics::color[4] = { 0.3294f, 0.4627f, 0.7490f, 1.0f };

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
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

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
	if (FAILED(hr)) { return hr; }
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
	pIDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
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
	vp.Width = (float)this->Width;
	vp.Height = (float)this->Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pDevContext->RSSetViewports(1, &vp);
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
	DXGI_MODE_DESC ds, dmatch;
	CComPtr<IDXGIOutput> output;
	pSwapChain->GetContainingOutput(&output.p);
	ds.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ds.RefreshRate.Numerator = 0;
	ds.RefreshRate.Denominator = 0;
	ds.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	ds.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	ds.Width = Width;
	ds.Height = Height;
	if (IsFullScreenMode){
		hr = pSwapChain->SetFullscreenState(FALSE, NULL);
		if (FAILED(hr))
			return false;
		hr = pSwapChain->ResizeTarget(&ds);
		if (FAILED(hr))
			return false;
		hr = pSwapChain->ResizeBuffers(BufferCount, Width, Height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);		
	}
	else
	{
		hr = pSwapChain->SetFullscreenState(TRUE, NULL);
		if (FAILED(hr))
			return false;
		hr = output->FindClosestMatchingMode(&ds, &dmatch, pDevice);
		if (FAILED(hr))
			return false;
		hr = pSwapChain->ResizeTarget(&dmatch);
		if (FAILED(hr))
			return false;
		hr = pSwapChain->ResizeBuffers(BufferCount, dmatch.Width, dmatch.Height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	}
	if (FAILED(hr))
		return false;
	hr = SetRenderTargets();
	SetViewPort();
	return hr;
}

void Graphics::SizeEvent()
{

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
