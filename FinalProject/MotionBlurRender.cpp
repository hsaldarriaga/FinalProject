#include "MotionBlurRender.h"
#include "FileLoader.h"
using namespace::DirectX;

MotionBlurRender::MotionBlurRender(Graphics* g)
{
	this->G = g;
	stride = sizeof(VERTEXENTRY);
	offset = 0;
}


HRESULT MotionBlurRender::Initialize(XMVECTOR Kd)
{
	if (!CreateBuffers(Kd))
		return E_FAIL;
	HRESULT hr = LoadShaders();
	ScreenSize = new ToTexture(G);
	ReduceSize = new ToTexture(G);
	RestoreSize = new ToTexture(G);
	hr = ScreenSize->Initialize();
	if (FAILED(hr)) { return hr; }
	hr = ReduceSize->Initialize();
	if (FAILED(hr)) { return hr; }
	hr = RestoreSize->Initialize();
	return hr;
}

bool MotionBlurRender::CreateBuffers(XMVECTOR Kd)
{
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
	if (FAILED(hr)) { return false; }
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
	if (FAILED(hr)) { return false; }

	ZeroMemory(&bufferdesc, sizeof(D3D11_BUFFER_DESC));
	bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferdesc.ByteWidth = sizeof(XMMATRIX);
	bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferdesc.MiscFlags = 0;
	bufferdesc.StructureByteStride = 0;
	bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = G->pDevice->CreateBuffer(&bufferdesc, NULL, &cbMatrix.p);
	if (FAILED(hr)) { return false; }
	bufferdesc.ByteWidth = sizeof(XMVECTOR);
	hr = G->pDevice->CreateBuffer(&bufferdesc, NULL, &cbSize.p);
	if (FAILED(hr)) { return false; }
	hr = G->pDevice->CreateBuffer(&bufferdesc, NULL, &cbColorToBlur);
	if (FAILED(hr)) { return false; }
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
	hr = G->pDevice->CreateSamplerState(&samplerdesc, &sampler.p);
	if (FAILED(hr))
		return false;
	D3D11_MAPPED_SUBRESOURCE ss;
	hr = G->pDevContext->Map(cbColorToBlur, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	if (FAILED(hr)) { return false; }
	XMFLOAT4 Kdstruct; XMStoreFloat4(&Kdstruct, Kd);
	memcpy(ss.pData, &Kdstruct, sizeof(XMFLOAT4));
	G->pDevContext->Unmap(cbColorToBlur, 0);
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;
	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = G->pDevice->CreateBlendState(&desc, &BlendState.p);
	if (FAILED(hr)) { return false; }
	return true;
}

bool MotionBlurRender::UpdateBuffers(Camera* Cmr)
{
	ScreenSize->SizeChanged((float)G->Width, (float)G->Height);
	ReduceSize->SizeChanged(G->Width / 3.0f, G->Height / 3.0f);
	RestoreSize->SizeChanged((float)G->Width, (float)G->Height);
	UINT vertexcount = IndexCount;
	//intentar on vector
	float left = (float)(G->Width / -2.0f);
	float top = (float)(G->Height / 2.0f);
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
	memcpy(ss.pData, vertexdata, sizeof(VERTEXENTRY)* ARRAYSIZE(vertexdata));
	G->pDevContext->Unmap(VertexBuffer.p, 0);
	hr = G->pDevContext->Map(cbMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	XMMATRIX mat = XMMatrixLookAtLH(XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, -1.0f)), XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 1.0f)), Cmr->Up) * XMMatrixOrthographicLH((float)G->Width, (float)G->Height, 0.0f, 100.0f);
	memcpy(ss.pData, &mat, sizeof(XMMATRIX));
	G->pDevContext->Unmap(cbMatrix, 0);
	return true;
}

HRESULT MotionBlurRender::LoadShaders()
{
	ID3D11Device* g = G->pDevice;
	BYTE* Data;
	LONG Size;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/HorizontalBlurVertexShader.cso", &Data, &Size)))
		return E_FAIL;
	D3D11_INPUT_ELEMENT_DESC desc[3];
	desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	desc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (FAILED(g->CreateInputLayout(desc, 2, Data, Size, &InputLayout.p))) {
		delete[] Data;
		return E_FAIL;
	}
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VSHorizontalBlur.p))) {
		delete[] Data;
		return E_FAIL;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/VerticalBlurVertexShader.cso", &Data, &Size)))
		return E_FAIL;
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VSVerticalBlur.p))) {
		delete[] Data;
		return E_FAIL;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/HorizontalVerticalBlurPixelShader.cso", &Data, &Size)))
		return E_FAIL;
	HRESULT hr = g->CreatePixelShader(Data, Size, NULL, &PSBlurShader.p);
	delete[] Data;
	return hr;
}

void MotionBlurRender::Prepare()
{
	ScreenSize->StartToDraw3DAllHere();
}

void MotionBlurRender::Render()
{
	ID3D11DeviceContext* d = G->pDevContext;
	for (UINT i = 0; i < 3; i++)
	{
		//Reduce texture size a half
		ReduceSize->RenderResourceToTexture(ScreenSize->getResultantTexture());
		//End reduce
		ID3D11ShaderResourceView * reducescreen = ReduceSize->getResultantTexture();
		ReduceSize->StartToDraw2DAllHere();
		d->IASetInputLayout(InputLayout);
		d->IASetVertexBuffers(0, 1, &VertexBuffer.p, &stride, &offset);
		d->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		d->VSSetShader(VSHorizontalBlur, NULL, 0);
		d->VSSetConstantBuffers(0, 1, &cbMatrix.p);
		D3D11_MAPPED_SUBRESOURCE ss;
		d->Map(cbSize, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
		CONSTANTFLOAT v = CONSTANTFLOAT((float)G->Width);
		memcpy(ss.pData, &v, sizeof(CONSTANTFLOAT));
		d->Unmap(cbSize, 0);
		d->VSSetConstantBuffers(1, 1, &cbSize.p);

		d->PSSetShader(PSBlurShader, NULL, 0);
		d->PSSetConstantBuffers(0, 1, &cbColorToBlur.p);
		d->PSSetSamplers(0, 1, &sampler.p);
		d->PSSetShaderResources(0, 1, &reducescreen);

		d->DrawIndexed(IndexCount, 0, 0);

		reducescreen = ReduceSize->getResultantTexture();
		ReduceSize->StartToDraw2DAllHere();
		d->VSSetShader(VSVerticalBlur, NULL, 0);
		d->VSSetConstantBuffers(0, 1, &cbMatrix.p);

		d->Map(cbSize, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
		v = CONSTANTFLOAT((float)G->Height);
		memcpy(ss.pData, &v, sizeof(CONSTANTFLOAT));
		d->Unmap(cbSize, 0);
		d->VSSetConstantBuffers(1, 1, &cbSize.p);

		d->PSSetShader(PSBlurShader, NULL, 0);
		d->PSSetConstantBuffers(0, 1, &cbColorToBlur.p);
		d->PSSetSamplers(0, 1, &sampler.p);
		d->PSSetShaderResources(0, 1, &reducescreen);

		d->DrawIndexed(IndexCount, 0, 0);
		ScreenSize->RenderResourceToTexture(ReduceSize->getResultantTexture());
	}

	d->OMSetBlendState(BlendState, NULL, 0xffffffff);
	RestoreSize->RenderResourceToScreen(ReduceSize->getResultantTexture());
	d->OMSetBlendState(NULL, NULL, 0xffffffff);
}

MotionBlurRender::~MotionBlurRender()
{
	if (ScreenSize)
		delete ScreenSize;
	if (ReduceSize)
		delete ReduceSize;
	if (RestoreSize)
		delete RestoreSize;
}
