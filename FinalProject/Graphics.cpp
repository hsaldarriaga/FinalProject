#include "Graphics.h"


Graphics::Graphics(HWND hWnd, int Width, int Height)
{
	this->hWnd = hWnd;
	this->Width = Width;
	this->Height = Height;
	this->IsReady = false; //Está función indica si se puede renderizar o no
	this->BufferCount = 1;
}

const float Graphics::color[4] = { 0.3294f, 0.4627f, 0.7490f, 1.0f }; //color azul

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
	sd.BufferCount = 1; // como se ejecuta en modo ventana, solo hay 1 sólo Back Buffer
	sd.BufferDesc.Width = this->Width;
	sd.BufferDesc.Height = this->Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //formato de 4 enteros sin signo RGBA
	sd.BufferDesc.RefreshRate.Numerator = 60;//Tasa de refresco de la pantalla, 60 fotogramas por segundo
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;//HANDLE que corresponde a la ventana creada
	sd.SampleDesc.Count = SAMPLE_COUNT;//Parámetros que definen el nivel de Anti-Aliasing(AA)
	sd.SampleDesc.Quality = QUALITY_LEVEL;
	sd.Windowed = TRUE;//Se ejecuta en modo ventana
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//Permite cambiar a pantalla completa

	D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0};//Solo inicia la aplicación si la tarjeta
	//de video es compatible con DirectX 11
	D3D_FEATURE_LEVEL outLevel;
	HRESULT hr = D3D11CreateDevice(
		NULL,//Se mostrar el contenido en la pantalla por defecto
		D3D_DRIVER_TYPE_HARDWARE, //Los drivers de la tarjeta de video serán usados.
		NULL,
#if _DEBUG
		D3D11_CREATE_DEVICE_DEBUG, //Permite mostrar mensajes en la consola en caso de errores.
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
	//Desde aquí
	CComPtr<IDXGIDevice> pDXGIDevice;
	hr = pDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));
	if (FAILED(hr)) { return hr; }
	CComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	if (FAILED(hr)) { return hr; }
	CComPtr<IDXGIFactory> pIDXGIFactory;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));
	if (FAILED(hr)) { return hr; }
	//Hasta acá son una serie de querys para llegar desde ID3D11Device a IDXGIFactory 
	//Para poder crear el SwapChain
	hr = pIDXGIFactory->CreateSwapChain(pDevice, &sd, &pSwapChain);
	//Esto quiere decir que la aplicación va a gestionar todo el proceso de pasar de pantalla completa
	// a modo ventana y viceversa de forma manual
	pIDXGIFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
	return hr;
}

HRESULT Graphics::SetRenderTargets()
{
	CComPtr<ID3D11Texture2D> pBackBuffer, DepthBuffer;
	//Se obtiene el back buffer a partir del SwapChain
	HRESULT hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(hr))
		return hr;
	//Se llena la estructura para crear el Depthbuffer
	D3D11_TEXTURE2D_DESC ds; 
	ZeroMemory(&ds, sizeof(D3D11_TEXTURE2D_DESC));
	ds.MipLevels = 1;
	ds.ArraySize = 1;
	ds.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ds.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ds.Height = this->Height;
	ds.Width = this->Width;
	ds.SampleDesc.Count = SAMPLE_COUNT;
	ds.SampleDesc.Quality = QUALITY_LEVEL;
	ds.Usage = D3D11_USAGE_DEFAULT;
	hr = pDevice->CreateTexture2D(&ds, NULL, &DepthBuffer); //Se crea la textura que representa el Depth Buffer
	if (FAILED(hr))
		return hr;
	//pDepthStencilView es la interface que representa el DepthBuffer
	hr = pDevice->CreateDepthStencilView(DepthBuffer, NULL, &pDepthStencilView);
	if (FAILED(hr))
		return hr;
	//pRenderTargetView Representa el backbuffer
	hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	if (FAILED(hr))
		return hr;
	ID3D11RenderTargetView* rtv[1] = { pRenderTargetView };
	//Una vez creados se asignan a la graphic pipeline
	pDevContext->OMSetRenderTargets(1, rtv, pDepthStencilView);
	return hr;
}

void Graphics::SetViewPort()
{
	//Se define el área de dibujo
	vp.Width = (float)this->Width;
	vp.Height = (float)this->Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//El área de dibujo será toda el área definida para dibujar.
	pDevContext->RSSetViewports(1, &vp);
}


HRESULT Graphics::SwitchMode()
{
	IsReady = false;
	BOOL IsFullScreenMode;
	//Función que indica si se encuentra en Pantalla completa o modo ventana
	HRESULT hr = pSwapChain->GetFullscreenState(&IsFullScreenMode, NULL);
	if (FAILED(hr))
		return hr;
	//Se liberan el backbuffer y el depth buffer y se eliminan.
	pDevContext->OMSetRenderTargets(0, 0, 0);
	pRenderTargetView.Release();
	pDepthStencilView.Release();
	int BufferCount = 1;
	//Si estaba en Pantalla completa, regresa al tamaño original
	if (IsFullScreenMode) 
	{
		RECT screen = { 0, 0, 800, 600 };
		AdjustWindowRect(&screen, WS_OVERLAPPEDWINDOW, FALSE);
		Width = screen.right - screen.left;
		Height = screen.bottom - screen.top;
	}
	else 
	{
		//Se asigna el tamaño de la aplicación el mismo que el de la pantalla para que funcione
		// a pantalla completa y se activa el TripleBuffer
		Width = 1366;
		Height = 768;
		BufferCount = 3;
	}
	DXGI_MODE_DESC ds, dmatch;
	CComPtr<IDXGIOutput> output;
	pSwapChain->GetContainingOutput(&output.p);
	ds.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ds.RefreshRate.Numerator = 0;//Conserva la tasa de refresco que tenía antes
	ds.RefreshRate.Denominator = 0;
	ds.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;//Conserva el modo que tenía antes
	ds.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //Conserva el modo que tenía antes
	//Se reasigna el nuevo ancho y alto y el resto se mantiene igual.	
	ds.Width = Width;
	ds.Height = Height;
	if (IsFullScreenMode)
	{
		//Se pasa a modo ventana
		hr = pSwapChain->SetFullscreenState(FALSE, NULL);
		if (FAILED(hr))
			return false;
		//Se redimensiona la superficie donde se pinta el contenido.
		hr = pSwapChain->ResizeTarget(&ds);
		if (FAILED(hr))
			return false;
		//Se redimensionan el back y depthbuffer para que se ajusten al nuevo tamaño
		hr = pSwapChain->ResizeBuffers(BufferCount, Width, Height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);		
	}
	else
	{
		hr = pSwapChain->SetFullscreenState(TRUE, NULL);
		if (FAILED(hr))
			return false;
		//A partir de la resolución y la frecuencia de actualización que está en 'ds' busca la que mejor coincida con la pantalla
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
	//vuelve y se asígnan pRenderTargetView y pDepthStencilView
	hr = SetRenderTargets();
	//Se asigna el área de pintado también
	SetViewPort();
	return hr;
}

void Graphics::SizeEvent()
{

}

void Graphics::Release() 
{
	//Antes de eliminar los gráficos se debe regresar a modo ventana.
	pSwapChain->SetFullscreenState(FALSE, NULL);
}

void Graphics::Ready(bool IsReady)
{
	this->IsReady = IsReady;
}

Graphics::~Graphics()
{
}
