#include "ToTexture.h"
#include "FileLoader.h"
using namespace::DirectX;

ToTexture::ToTexture(Graphics* g)
{
	this->G = g;
	black[0] = black[1] = black[2] = 0;
	black[3] = 1.0f;
}

HRESULT ToTexture::Initialize()
{
	HRESULT hr = CreatePipelineResources();
	if (FAILED(hr)) { return hr; }
	hr = CreateVertexShaderBuffers();
	if (FAILED(hr)) { return hr; }
	hr = CreatePixelShaderBuffers();
	return hr;
}

HRESULT ToTexture::SizeChanged(float Width, float Height)
{
	HRESULT hr;
	if (this->Width != Width || this->Height != Height) {
		this->Height = Height;
		this->Width = Width;
		hr = InitializeRTVAndSRV();
		if (FAILED(hr)) { return hr; }
		hr = SetTargetMatrix();
		if (FAILED(hr)) { return hr; }
		hr = SetBuffers();
		SetViewPort();
		if (FAILED(hr)) { return hr; }
	}
	return ERROR_SUCCESS;
}

HRESULT ToTexture::SizeChangedOnlyBuffers(float TargetWidth, float TargetHeight)
{
	HRESULT hr;
	if (this->Width != TargetWidth || this->Height != TargetHeight) {
		this->Height = TargetHeight;
		this->Width = TargetWidth;
		hr = SetTargetMatrix();
		if (FAILED(hr)) { return hr; }
		hr = SetBuffers();
		if (FAILED(hr)) { return hr; }
	}
	return ERROR_SUCCESS;
}

HRESULT ToTexture::InitializeRTVAndSRV()
{
	if (TextureRenderTargetView)
		TextureRenderTargetView.Release();
	if (RenderShaderResource)
		RenderShaderResource.Release();
	if (TargetViewTexture2d)
		TargetViewTexture2d.Release();
	if (ShaderResourceTexture)
		ShaderResourceTexture.Release();
	D3D11_TEXTURE2D_DESC texdesc;
	ZeroMemory(&texdesc, sizeof(D3D11_TEXTURE2D_DESC));
	texdesc.Width = (UINT)Width;
	texdesc.Height = (UINT)Height;
	texdesc.MipLevels = 1;
	texdesc.ArraySize = 1;
	texdesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texdesc.SampleDesc.Count = 8;
	texdesc.SampleDesc.Quality = 16;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texdesc.CPUAccessFlags = 0;
	texdesc.MiscFlags = 0;
	HRESULT hr = G->pDevice->CreateTexture2D(&texdesc, NULL, &TargetViewTexture2d.p);
	if (FAILED(hr)) { return hr; }
	texdesc.SampleDesc.Count = 1;
	texdesc.SampleDesc.Quality = 0;
	texdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	hr = G->pDevice->CreateTexture2D(&texdesc, NULL, &ShaderResourceTexture.p);
	if (FAILED(hr)) { return hr; }
	D3D11_RENDER_TARGET_VIEW_DESC tviewdesc;
	tviewdesc.Format = texdesc.Format;
	tviewdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	tviewdesc.Texture2D.MipSlice = 0;
	hr = G->pDevice->CreateRenderTargetView(TargetViewTexture2d, &tviewdesc, &TextureRenderTargetView.p);
	if (FAILED(hr)) { return hr; }
	D3D11_SHADER_RESOURCE_VIEW_DESC resdesc;
	resdesc.Format = texdesc.Format;
	resdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resdesc.Texture2D.MostDetailedMip = 0;
	resdesc.Texture2D.MipLevels = 1;
	hr = G->pDevice->CreateShaderResourceView(ShaderResourceTexture, &resdesc, &RenderShaderResource.p);
	return hr;
}

HRESULT ToTexture::CreateVertexShaderBuffers()
{
	ID3D11Device* g = G->pDevice;
	BYTE* Data;
	LONG Size;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/SimpleVertexShader.cso", &Data, &Size)))
		return E_FAIL;
	D3D11_INPUT_ELEMENT_DESC desc[3];
	desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	desc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (FAILED(g->CreateInputLayout(desc, 2, Data, Size, &InputLayout.p))) {
		delete[] Data;
		return E_FAIL;
	}
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VSShader.p))) {
		delete[] Data;
		return E_FAIL;
	}
	delete[] Data;

	IndexCount = 6;
	UINT vertexcount = IndexCount;
	D3D11_BUFFER_DESC bufferdesc;
	ZeroMemory(&bufferdesc, sizeof(D3D11_BUFFER_DESC));
	bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferdesc.ByteWidth = sizeof(VERTEXENTRY)* vertexcount;
	bufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = G->pDevice->CreateBuffer(&bufferdesc, NULL, &VertexBuffer.p);
	if (FAILED(hr)) { return hr; }

	bufferdesc.ByteWidth = sizeof(UINT)* IndexCount;
	bufferdesc.Usage = D3D11_USAGE_DEFAULT;
	bufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferdesc.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
	UINT* IndexData = new UINT[IndexCount];
	for (UINT i = 0; i < IndexCount; i++)
		IndexData[i] = i;
	D3D11_SUBRESOURCE_DATA data; ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = IndexData;
	hr = G->pDevice->CreateBuffer(&bufferdesc, &data, &IndexBuffer.p);
	delete[] IndexData;
	if (FAILED(hr)) { return hr; }

	ZeroMemory(&bufferdesc, sizeof(D3D11_BUFFER_DESC));
	bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferdesc.ByteWidth = sizeof(XMMATRIX);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = G->pDevice->CreateBuffer(&bufferdesc, NULL, &cbMatrix.p);
	if (FAILED(hr)) { return hr; }

	return hr;
}

HRESULT ToTexture::CreatePixelShaderBuffers()
{
	ID3D11Device* g = G->pDevice;
	BYTE* Data;
	LONG Size;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/SimplePixelShader.cso", &Data, &Size)))
		return E_FAIL;
	if (FAILED(g->CreatePixelShader(Data, Size, NULL, &PSShader.p))) {
		delete[] Data;
		return E_FAIL;
	}
	delete[] Data;

	D3D11_SAMPLER_DESC samplerdesc;
	ZeroMemory(&samplerdesc, sizeof(D3D11_SAMPLER_DESC));
	samplerdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerdesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerdesc.MipLODBias = 0.0f;
	samplerdesc.MaxAnisotropy = 1;
	samplerdesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerdesc.BorderColor[0] = samplerdesc.BorderColor[1] = samplerdesc.BorderColor[2] = samplerdesc.BorderColor[3] = 0.0f;
	samplerdesc.MinLOD = 0;
	samplerdesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = G->pDevice->CreateSamplerState(&samplerdesc, &sampler.p);
	if (FAILED(hr)) { return hr; };

	return hr;
}

HRESULT ToTexture::CreatePipelineResources()
{
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	ZeroMemory(&depthDisabledStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	HRESULT hr = G->pDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &ZBufferOff.p);
	return hr;
}

HRESULT ToTexture::SetBuffers()
{
	UINT vertexcount = IndexCount;
	//intentar on vector
	float left = (float)(Width / -2.0f);
	float top = (float)(Height / 2.0f);
	float right = -left;
	float bottom = -top;

	VERTEXENTRY vertexdata[] = {
		//First Triangle
		VERTEXENTRY(left, top, 0.0f, 0.0f, 0.0f),
		VERTEXENTRY(right, bottom, 0.0f, 1.0f, 1.0f),
		VERTEXENTRY(left, bottom, 0.0f, 0.0f, 1.0f),
		//Second Triangle
		VERTEXENTRY(left, top, 0.0f, 0.0f, 0.0f),
		VERTEXENTRY(right, top, 0.0f, 1.0f, 0.0f),
		VERTEXENTRY(right, bottom, 0.0f, 1.0f, 1.0f)
	};
	D3D11_MAPPED_SUBRESOURCE ss;
	HRESULT hr = G->pDevContext->Map(VertexBuffer.p, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
    if(FAILED(hr)) { return hr; }
	memcpy(ss.pData, vertexdata, sizeof(VERTEXENTRY)* ARRAYSIZE(vertexdata));
	G->pDevContext->Unmap(VertexBuffer.p, 0);
	return hr;
}

HRESULT ToTexture::SetTargetMatrix()
{
	D3D11_MAPPED_SUBRESOURCE ss;
	HRESULT hr = G->pDevContext->Map(cbMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	if (FAILED(hr)) { return hr; }
	XMMATRIX mat = XMMatrixLookAtLH(XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, -1.0f)), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 1.0f)), XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f))) * XMMatrixOrthographicLH((float)Width, (float)Height, 0.0f, 100.0f);
	memcpy(ss.pData, &mat, sizeof(XMMATRIX));
	G->pDevContext->Unmap(cbMatrix, 0);
	return hr;
}

void ToTexture::SetViewPort()
{
	Viewport.Height = Height;
	Viewport.Width = Width;
	Viewport.MaxDepth = 1.0f;
	Viewport.MinDepth = 0.0f;
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
}

ID3D11ShaderResourceView* ToTexture::getResultantTexture()
{
	G->pDevContext->ResolveSubresource(ShaderResourceTexture, 0, TargetViewTexture2d, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	return RenderShaderResource.p;
}
/*
	Dont Forget To Resolve Subresources
*/
void ToTexture::RenderResourceToTexture(ID3D11ShaderResourceView* Texsrc)
{
	ID3D11DeviceContext* d = G->pDevContext;
	d->OMSetRenderTargets(1, &TextureRenderTargetView.p, NULL);
	d->OMSetDepthStencilState(ZBufferOff, 1);

	d->IASetInputLayout(InputLayout);
	UINT stride = sizeof(VERTEXENTRY), offset = 0;
	d->IASetVertexBuffers(0, 1, &VertexBuffer.p, &stride, &offset);
	d->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VSShader, NULL, 0);
	d->VSSetConstantBuffers(0, 1, &cbMatrix.p);

	d->PSSetShader(PSShader, NULL, 0);
	d->PSSetSamplers(0, 1, &sampler.p);
	d->PSSetShaderResources(0, 1, &Texsrc);
	d->RSSetViewports(1, &Viewport);
	d->DrawIndexed(IndexCount, 0, 0);

}

void ToTexture::RenderResourceToScreen(ID3D11ShaderResourceView* Texsrc)
{
	ID3D11DeviceContext* d = G->pDevContext;
	d->OMSetRenderTargets(1, &G->pRenderTargetView.p, NULL);
	d->OMSetDepthStencilState(ZBufferOff, 1);

	d->IASetInputLayout(InputLayout);
	UINT stride = sizeof(VERTEXENTRY), offset = 0;
	d->IASetVertexBuffers(0, 1, &VertexBuffer.p, &stride, &offset);
	d->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VSShader, NULL, 0);
	d->VSSetConstantBuffers(0, 1, &cbMatrix.p);

	d->PSSetShader(PSShader, NULL, 0);
	d->PSSetSamplers(0, 1, &sampler.p);
	d->PSSetShaderResources(0, 1, &Texsrc);

	G->SetViewPort();

	d->DrawIndexed(IndexCount, 0, 0);

	d->OMSetDepthStencilState(NULL, 1);
}
/*
	The RenderTargetView Must Have the same size as the DepthStencilView Used
*/
void ToTexture::StartToDraw3DAllHere()
{
	G->pDevContext->OMSetRenderTargets(1, &TextureRenderTargetView.p, G->pDepthStencilView);
	G->pDevContext->ClearRenderTargetView(TextureRenderTargetView, black);
	G->pDevContext->ClearDepthStencilView(G->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ToTexture::StartToDraw2DAllHere()
{
	G->pDevContext->OMSetRenderTargets(1, &TextureRenderTargetView.p, NULL);
	G->pDevContext->ClearRenderTargetView(TextureRenderTargetView, G->color);
	G->pDevContext->OMSetDepthStencilState(ZBufferOff, 1);
}

ToTexture::~ToTexture()
{
}
