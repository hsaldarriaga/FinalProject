#pragma once
#include <Windows.h>
#include "Graphics.h"
#include <wincodec.h>

class ImageFactory
{
public:
	ImageFactory(ID3D11Device* pDevice);
	HRESULT getImageFromFileName(const char* filename, ID3D11ShaderResourceView** view);
	~ImageFactory();
private: 
	HRESULT createTexture(IWICBitmapFrameDecode* pFrame, ID3D11ShaderResourceView** view);
	DXGI_FORMAT getDXGIFormat(WICPixelFormatGUID WICFormat);
	UINT getbpp(IWICImagingFactory* factory, REFGUID guid);
	ID3D11Device* pDevice;
	IWICImagingFactory* pFactory = NULL;
};

