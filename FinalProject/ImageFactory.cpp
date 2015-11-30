#include "ImageFactory.h"
#include "WICHelpers.h"

ImageFactory::ImageFactory(ID3D11Device* device)
{
	this->pDevice = device;
	this->pFactory = NULL;
}
//El código de está clase fue tomado de https://msdn.microsoft.com/en-us/library/windows/desktop/ff476904(v=vs.85).aspx
HRESULT ImageFactory::getImageFromFileName(const char* filename, ID3D11ShaderResourceView** view)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pFrame = NULL;
	HRESULT hr = ERROR_SUCCESS;
	//Se inicializa los objetos COM necesarios para leer una textura
	if (!pFactory) 
	{
		hr = CoInitialize(NULL);
		if (FAILED(hr))
			return hr;
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
		}
	if (SUCCEEDED(hr))
	{
		size_t size = strlen(filename) + 1;
		wchar_t* wa = new wchar_t[size];
		//Convierte de char* a wchar_t*
		mbstowcs_s(NULL, wa, size, filename, size);
		//Se obtiene la imagén a partir de la ruta filename
		hr = pFactory->CreateDecoderFromFilename(wa, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
		if (SUCCEEDED(hr))
		{
			//Se obtiene la imagen
			hr = pDecoder->GetFrame(0, &pFrame);
			if (SUCCEEDED(hr)) {
				//Se crea la textura a partir de la imagen
				hr = createTexture(pFrame, view);
			}
		}
		delete[] wa;
	}
	if (pFrame)
		pFrame->Release();
	if (pDecoder)
	pDecoder->Release();
	return hr;
}

HRESULT ImageFactory::createTexture(IWICBitmapFrameDecode* pFrame, ID3D11ShaderResourceView** view)
{
	ID3D11Texture2D* tex = NULL;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	HRESULT hr = pFrame->GetSize(&desc.Width, &desc.Height);
	if (FAILED(hr))
		return hr;
	desc.MipLevels = desc.ArraySize = 1;
	WICPixelFormatGUID WICformat;
	hr = pFrame->GetPixelFormat(&WICformat);
	if (FAILED(hr))
		return hr;
	desc.Format = getDXGIFormat(WICformat); // Se obtiene el formato equivalente de WIC a DXGI
	UINT bpp = 0;
	WICPixelFormatGUID convertGUID;
	bool otherformat = false;
	if (desc.Format == DXGI_FORMAT_UNKNOWN)
	{
		for (size_t i = 0; i < 41; ++i)
		{
			if (memcmp(&g_WICConvert[i].source, &WICformat, sizeof(WICPixelFormatGUID)) == 0)
			{
				memcpy(&convertGUID, &g_WICConvert[i].target, sizeof(WICPixelFormatGUID));

				desc.Format = getDXGIFormat(g_WICConvert[i].target);
				bpp = getbpp(pFactory, convertGUID);//Bits Per Pixel
				otherformat = true;
				break;
			}
		}
		if (desc.Format == DXGI_FORMAT_UNKNOWN) //Si no se encontró el formato en DXGI
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}
	else {
		bpp = getbpp(pFactory, WICformat);
		
	}
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //Indica que es una textura que leerá un sombreador
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;//La CPU puede modificar la textura
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	if (bpp == 0)
		return E_FAIL;
	size_t rowPitch = (desc.Width * bpp + 7) / 8;
	size_t imageSize = rowPitch * desc.Height;
	data.SysMemPitch = static_cast<UINT>(rowPitch);
	data.SysMemSlicePitch = static_cast<UINT>(imageSize);
	BYTE *imagedata = new BYTE[imageSize];
	if (otherformat)
	{
		IWICImagingFactory* pWIC = pFactory;
		if (!pWIC)
			return E_NOINTERFACE;

		IWICFormatConverter* FC;
		hr = pWIC->CreateFormatConverter(&FC);
		if (FAILED(hr)){
			FC->Release(); return hr;
		}
		hr = FC->Initialize(pFrame, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)){
			FC->Release(); return hr;
		}
		//Se obtienen los pixeles de la imagen y se almacenan en imagedata
		hr = FC->CopyPixels(0, data.SysMemPitch, data.SysMemSlicePitch, imagedata);
		if (FAILED(hr)){
			FC->Release(); return hr;
		}
		FC->Release();
	} 
	else 
	{
		hr = pFrame->CopyPixels(0, data.SysMemPitch, data.SysMemSlicePitch, imagedata);
	}
	if (FAILED(hr))
		return hr;
	data.pSysMem = imagedata;
	//Una vez llenados todos los campos de Data, se crea la textura
	hr = pDevice->CreateTexture2D(&desc, &data, &tex);
	delete[] imagedata;
	if (SUCCEEDED(hr)) 
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
		ZeroMemory(&viewdesc, sizeof(viewdesc));
		viewdesc.Format = desc.Format;
		viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewdesc.Texture2D.MipLevels = 1; // luego se crea el Shader Resource View que es la interface para poder asignar la textura al sombreador
		hr = pDevice->CreateShaderResourceView(tex, &viewdesc, view);
	}
	if (tex)
		tex->Release();
	return hr;
}
DXGI_FORMAT ImageFactory::getDXGIFormat(WICPixelFormatGUID WICFormat)
{
	if (WICFormat == GUID_WICPixelFormat128bppRGBAFloat)
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	if (WICFormat == GUID_WICPixelFormat64bppRGBAHalf)
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	if (WICFormat == GUID_WICPixelFormat64bppRGBA)
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppRGBA)
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppBGRA)
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppBGR)
		return DXGI_FORMAT_B8G8R8X8_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppRGBA1010102XR)
		return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppRGBA1010102)
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppRGBE)
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	if (WICFormat == GUID_WICPixelFormat16bppBGRA5551)
		return DXGI_FORMAT_B5G5R5A1_UNORM;
	if (WICFormat == GUID_WICPixelFormat16bppBGR565)
		return DXGI_FORMAT_B5G6R5_UNORM;
	if (WICFormat == GUID_WICPixelFormat32bppGrayFloat)
		return DXGI_FORMAT_R32_FLOAT;
	if (WICFormat == GUID_WICPixelFormat16bppGrayHalf)
		return DXGI_FORMAT_R16_FLOAT;
	if (WICFormat == GUID_WICPixelFormat16bppGray)
		return DXGI_FORMAT_R16_UNORM;
	if (WICFormat == GUID_WICPixelFormat8bppGray)
		return DXGI_FORMAT_R8_UNORM;
	if (WICFormat == GUID_WICPixelFormat8bppAlpha)
		return DXGI_FORMAT_A8_UNORM;
	if (WICFormat == GUID_WICPixelFormat96bppRGBFloat)
		return DXGI_FORMAT_R32G32B32_FLOAT;
	return DXGI_FORMAT_UNKNOWN;
}

UINT ImageFactory::getbpp(IWICImagingFactory* factory, REFGUID guid)
{
	IWICComponentInfo* cinfo = NULL;
	HRESULT hr = factory->CreateComponentInfo(guid, &cinfo);
	WICComponentType type;
	IWICPixelFormatInfo* pixelinfo = NULL;
	UINT bpp = 0;
	if (SUCCEEDED(hr)) 
	{
		hr = cinfo->GetComponentType(&type);
		if (SUCCEEDED(hr))
		{
			hr = cinfo->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(&pixelinfo));
			if (SUCCEEDED(hr))
			{
				hr = pixelinfo->GetBitsPerPixel(&bpp);
			}
		}
	}
	if (pixelinfo)
		pixelinfo->Release();
	if (cinfo)
		cinfo->Release();
	if (SUCCEEDED(hr))
		return bpp;
	else
		return 0;
}

ImageFactory::~ImageFactory()
{
	if (pFactory)
		pFactory->Release();
	CoUninitialize();
}
