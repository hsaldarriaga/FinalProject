#pragma once
#include "ToTexture.h"
#include "Camera.h"
class MotionBlurRender
{
public:
	MotionBlurRender(Graphics*);
	HRESULT Initialize(DirectX::XMVECTOR Kd);
	void Prepare();
	bool UpdateBuffers(Camera* Cmr);
	void Render();
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
	~MotionBlurRender();
private:
	HRESULT LoadShaders();
	bool CreateBuffers(DirectX::XMVECTOR Kd);

	Graphics* G;
	ToTexture* ScreenSize = NULL, *ReduceSize = NULL, *RestoreSize = NULL;

	CComPtr<ID3D11BlendState> BlendState;
	CComPtr<ID3D11Buffer> VertexBuffer = NULL, IndexBuffer = NULL;
	CComPtr<ID3D11InputLayout> InputLayout;
	CComPtr<ID3D11Buffer> cbMatrix, cbSize, cbColorToBlur;
	CComPtr<ID3D11SamplerState> sampler;
	CComPtr<ID3D11VertexShader> VSVerticalBlur, VSHorizontalBlur;
	CComPtr<ID3D11PixelShader> PSBlurShader;
	UINT stride, offset, IndexCount;
};