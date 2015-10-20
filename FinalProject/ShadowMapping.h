#pragma once

#include "IMesh.h"
#include"Camera.h"


#define CASCADE_COUNT 3

class ShadowMapping
{
	//Cascade Shadow Mapping With Percentage Closer filtering
public:
	ShadowMapping(Graphics*, DirectX::XMFLOAT2[], Camera* Cmr);
	bool Initialize();
	void SetParameters(DirectX::XMFLOAT3 LightEye, DirectX::XMFLOAT3 LightTarget, DirectX::XMFLOAT3 LightUp);
	void AddBufferToShowShadows(Camera* Cmr);
	void Render(IMesh** Meshes, UINT count);
	DirectX::XMVECTOR getLighDir();
	DirectX::XMVECTOR getLighPos();
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
	~ShadowMapping();
private:
	bool CreateShadowBuffers();
	bool CreateComparision();
	bool CreateRasterizers();
	bool CreateShadowShaders();
	bool CreateShadowShaderBuffers();
	Graphics* G;
	UINT TextureSize = 2048;
	DirectX::XMVECTOR LightEye, LightTarget, LightUp;

	CComPtr<ID3D11DepthStencilView> shadowDepthStencilViews[CASCADE_COUNT];
	CComPtr<ID3D11RasterizerState> RasterizerDrawing;
	CComPtr<ID3D11RasterizerState> RasterizerShadow;
	CComPtr<ID3D11VertexShader> VertexShader;
	CComPtr<ID3D11PixelShader> PixelShader;
	CComPtr<ID3D11Buffer> cbMatrices;
	D3D11_VIEWPORT ViewPort;
	DirectX::XMMATRIX Orthogonalm[CASCADE_COUNT];
	DirectX::XMMATRIX View;
	float Intervals[CASCADE_COUNT];
	float MinInterval;
public:
	CComPtr<ID3D11ShaderResourceView> shaderResource;
	CComPtr<ID3D11SamplerState> sampler;
	CComPtr<ID3D11Buffer> cbShadowParams;
	CComPtr<ID3D11Buffer> cbLightMatrices;
};

