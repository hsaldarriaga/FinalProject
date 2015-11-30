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
	//Crea la textura a partir de una imagen pFrame y lo almacena en view
	HRESULT createTexture(IWICBitmapFrameDecode* pFrame, ID3D11ShaderResourceView** view);
	//Obtiene el formato de la imagen traducido a DXGI
	DXGI_FORMAT getDXGIFormat(WICPixelFormatGUID WICFormat);
	//Obtiene la cantidad de bytes por Píxel
	UINT getbpp(IWICImagingFactory* factory, REFGUID guid);
	ID3D11Device* pDevice;
	IWICImagingFactory* pFactory;


};

