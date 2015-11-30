#pragma once
#include "Graphics.h"
#include "Definitions.h"
class ToTexture
{
public:
	ToTexture(Graphics* g);
	HRESULT Initialize();
	HRESULT SizeChanged(float Targetwidth, float Targetheight);
	HRESULT SizeChangedOnlyBuffers(float TargetWidth, float TargetHeight);
	ID3D11ShaderResourceView* getResultantTexture();
	void RenderResourceToTexture(ID3D11ShaderResourceView*);
	void RenderResourceToScreen(ID3D11ShaderResourceView*);
	void StartToDraw3DAllHere();
	void StartToDraw2DAllHere();
	~ToTexture();

private:
	HRESULT InitializeRTVAndSRV();
	HRESULT CreateVertexShaderBuffers();
	HRESULT CreatePixelShaderBuffers();
	HRESULT CreatePipelineResources();
	HRESULT SetBuffers();
	HRESULT SetTargetMatrix();
	void SetViewPort();
	Graphics* G;
	UINT IndexCount;
	float Width, Height;

	CComPtr<ID3D11InputLayout> InputLayout;
	CComPtr<ID3D11DepthStencilState> ZBufferOff;
	CComPtr<ID3D11SamplerState> sampler;
	CComPtr<ID3D11Buffer> cbMatrix;
	CComPtr<ID3D11Buffer> VertexBuffer, IndexBuffer;
	CComPtr<ID3D11RenderTargetView> TextureRenderTargetView;
	CComPtr<ID3D11ShaderResourceView> RenderShaderResource;
	CComPtr<ID3D11Texture2D> TargetViewTexture2d, ShaderResourceTexture;
	CComPtr<ID3D11VertexShader> VSShader;
	CComPtr<ID3D11PixelShader> PSShader;
	D3D11_VIEWPORT Viewport;
	
	float black[4];
};

