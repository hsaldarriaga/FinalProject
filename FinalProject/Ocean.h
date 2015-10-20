#pragma once
#include "ocean_simulator.h"
#include "Definitions.h"

class Ocean
{
public:
	Ocean(Graphics* g);
	HRESULT Initialize();
	void SetLightsAndCameras(DirectX::XMVECTOR* LightDir, DirectX::XMVECTOR* CmrDir);
	void Render(DirectX::XMMATRIX*);
	~Ocean();
#pragma region Allocator
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
#pragma endregion
private:

	HRESULT InitializeBuffers();
	HRESULT InitializeShaderResources();

	OceanSimulator* simulator = NULL;
	Graphics* G;
	DirectX::XMMATRIX MLocal;
	UINT IndexCount;
	CComPtr<ID3D11InputLayout> InputLayout;
	CComPtr<ID3D11Buffer> VertexBuffer, IndexBuffer, MatricesBuffer, LightEyeBuffer;
	CComPtr<ID3D11VertexShader> VSShader;
	CComPtr<ID3D11PixelShader> PSShader;
	CComPtr<ID3D11SamplerState> samplerDisplacement, SamplerGradient;
};

