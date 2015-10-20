#include "ShadowMapping.h"
#include "Scene.h"
#include "FileLoader.h"
using namespace::DirectX;

ShadowMapping::ShadowMapping(Graphics* g, DirectX::XMFLOAT2 Zvalues[], Camera* Cmr)
{
	this->G = g;
	for (size_t i = 0; i < CASCADE_COUNT; i++)
	{
		Orthogonalm[i] = XMMatrixOrthographicLH((float)TextureSize, (float)TextureSize, Zvalues[i].x, Zvalues[i].y);
		Intervals[i] = Zvalues[i].y;
	}
	MinInterval = Zvalues[0].x;
}

bool ShadowMapping::Initialize()
{
	if (!CreateShadowBuffers())
		return false;
	if (!CreateComparision())
		return false;
	if (!CreateRasterizers())
		return false;
	if (!CreateShadowShaders())
		return false;
	if (!CreateShadowShaderBuffers())
		return false;
	ZeroMemory(&ViewPort, sizeof(D3D11_VIEWPORT));
	ViewPort.Height = (float)TextureSize;
	ViewPort.Width = (float)TextureSize;
	ViewPort.MinDepth = 0.0f;
	ViewPort.MaxDepth = 1.0f;
	return true;
}

bool ShadowMapping::CreateShadowBuffers()
{
	CComPtr<ID3D11Texture2D> textures;
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 3;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	shadowMapDesc.Height = TextureSize;
	shadowMapDesc.Width = TextureSize;
	HRESULT hr = G->pDevice->CreateTexture2D(&shadowMapDesc, NULL, &textures.p);
	if (FAILED(hr))
		return false;
	for (size_t i = 0; i < CASCADE_COUNT; i++)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depthStencilViewDesc.Texture2DArray.ArraySize = 1;
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
		hr = G->pDevice->CreateDepthStencilView(textures, &depthStencilViewDesc, &shadowDepthStencilViews[i].p);
		if (FAILED(hr))
			return false;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2DArray.ArraySize = CASCADE_COUNT;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	hr = G->pDevice->CreateShaderResourceView(textures, &shaderResourceViewDesc, &shaderResource.p);
	if (FAILED(hr))
		return false;
	return true;
}

bool ShadowMapping::CreateComparision()
{
	D3D11_SAMPLER_DESC samplerdesc;
	ZeroMemory(&samplerdesc, sizeof(D3D11_SAMPLER_DESC));
	samplerdesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerdesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerdesc.MipLODBias = 0.0f;
	samplerdesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerdesc.BorderColor[0] = samplerdesc.BorderColor[1] = samplerdesc.BorderColor[2] = samplerdesc.BorderColor[3] = 0.0f;
	samplerdesc.MinLOD = 0;
	samplerdesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = G->pDevice->CreateSamplerState(&samplerdesc, &sampler.p);
	if (FAILED(hr))
		return false;
	return true;
}

bool ShadowMapping::CreateRasterizers()
{
	D3D11_RASTERIZER_DESC rastdesc;
	ZeroMemory(&rastdesc, sizeof(D3D11_RASTERIZER_DESC));
	rastdesc.FillMode = D3D11_FILL_SOLID;
	rastdesc.CullMode = D3D11_CULL_BACK;
	rastdesc.MultisampleEnable = TRUE;
	rastdesc.DepthClipEnable = true;
	HRESULT hr = G->pDevice->CreateRasterizerState(&rastdesc, &RasterizerDrawing.p);
	if (FAILED(hr))
		return false;
	rastdesc.CullMode = D3D11_CULL_FRONT;
	hr = G->pDevice->CreateRasterizerState(&rastdesc, &RasterizerShadow.p);
	if (FAILED(hr))
		return false;
	return true;
}

bool ShadowMapping::CreateShadowShaders()
{
	BYTE* Data; 
	LONG Size;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/ShadowVertexShader.cso", &Data, &Size)))
		return false;
	HRESULT hr = G->pDevice->CreateVertexShader(Data, Size, NULL, &VertexShader.p);
	if (FAILED(hr)){
		delete[] Data;
		return false;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/ShadowPixelShader.cso", &Data, &Size)))
		return false;
	hr = G->pDevice->CreatePixelShader(Data, Size, NULL, &PixelShader.p);
	if (FAILED(hr)){
		delete[] Data;
		return false;
	}
	delete[] Data;
	return true;
}

bool ShadowMapping::CreateShadowShaderBuffers()
{
	ID3D11Device* g = G->pDevice;
	HRESULT hr;
	D3D11_BUFFER_DESC ds;
	ZeroMemory(&ds, sizeof(D3D11_BUFFER_DESC));
	ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ds.ByteWidth = sizeof(WORLD_VIEWxPROJ_BUFFER);
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.StructureByteStride = sizeof(WORLD_VIEWxPROJ_BUFFER);
	ds.Usage = D3D11_USAGE_DYNAMIC;
	hr = g->CreateBuffer(&ds, NULL, &cbMatrices);
	if (FAILED(hr)){ return false; }
	ds.ByteWidth = sizeof(XMFLOAT4);
	ds.StructureByteStride = sizeof(XMFLOAT4);
	hr = g->CreateBuffer(&ds, NULL, &cbShadowParams);
	if (FAILED(hr)){ return false; }
	ds.ByteWidth = sizeof(LIGHT_VIEW_PROJ_BUFFER);
	ds.StructureByteStride = sizeof(LIGHT_VIEW_PROJ_BUFFER);
	hr = g->CreateBuffer(&ds, NULL, &cbLightMatrices);
	if (FAILED(hr)){ return false; }
	return true;
}

void ShadowMapping::SetParameters(DirectX::XMFLOAT3 LightEye, DirectX::XMFLOAT3 LightTarget, DirectX::XMFLOAT3 LightUp)
{
	this->LightEye = XMLoadFloat3(&LightEye);
	this->LightTarget = XMLoadFloat3(&LightTarget);
	this->LightUp = XMLoadFloat3(&LightUp);
	View = XMMatrixTranspose(XMMatrixLookAtLH(this->LightEye, this->LightTarget, this->LightUp));
}

DirectX::XMVECTOR ShadowMapping::getLighPos()
{
	return LightEye;
}

DirectX::XMVECTOR ShadowMapping::getLighDir()
{
	return XMVector3Normalize(LightTarget - LightEye);
}

void ShadowMapping::AddBufferToShowShadows(Camera* Cmr)
{
	ID3D11DeviceContext* c = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE ss;
	c->Map(cbShadowParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	XMFLOAT4 vec = XMFLOAT4(Intervals[0], Intervals[1], Intervals[2], 0.0f);
	memcpy(ss.pData, &vec, sizeof(XMFLOAT4));
	c->Unmap(cbShadowParams, 0);
	c->Map(cbLightMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	LIGHT_VIEW_PROJ_BUFFER* ll = reinterpret_cast<LIGHT_VIEW_PROJ_BUFFER*>(ss.pData);
	ll->LightPos = LightEye;
	ll->LightView = XMMatrixTranspose(XMMatrixOrthographicLH((float)G->Width, (float)G->Height, MinInterval, Intervals[CASCADE_COUNT - 1]));
	ll->LightProj = XMMatrixTranspose(XMMatrixLookAtLH(LightEye, LightTarget, LightUp));
	c->Unmap(cbLightMatrices, 0);
}

void ShadowMapping::Render(IMesh** Meshes, UINT count)
{
	ID3D11DeviceContext* c = G->pDevContext;
	c->RSSetState(RasterizerShadow);
	c->RSSetViewports(1, &ViewPort);

	c->VSSetShader(VertexShader, NULL, 0);
	c->PSSetShader(PixelShader, NULL, 0);
	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	c->PSSetShaderResources(1, 1, pSRV);
	for (size_t j = 0; j < CASCADE_COUNT; j++)
	{
		c->ClearDepthStencilView(shadowDepthStencilViews[j], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		c->OMSetRenderTargets(0, NULL, shadowDepthStencilViews[j]);
		for (size_t i = 0; i < count; i++)
		{
			D3D11_MAPPED_SUBRESOURCE sub;
			c->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
			WORLD_VIEWxPROJ_BUFFER* data = reinterpret_cast<WORLD_VIEWxPROJ_BUFFER*>(sub.pData);
			data->World = DirectX::XMMatrixTranspose(*Meshes[i]->getMatrix());
			data->ViewProj = DirectX::XMMatrixTranspose(View * Orthogonalm[i]);
			c->Unmap(cbMatrices, 0);
			c->VSSetConstantBuffers(0, 1, &cbMatrices.p);
			c->IASetVertexBuffers(0, 1, &Meshes[i]->VertexBuffer.p, &Meshes[i]->stride, &Meshes[i]->offset);
			c->IASetIndexBuffer(Meshes[i]->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			c->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			c->DrawIndexed(Meshes[i]->getIndexesCount(), 0, 0);
			
		}
	}
	G->pDevContext->OMSetRenderTargets(1, &G->pRenderTargetView.p, G->pDepthStencilView);
	c->RSSetState(RasterizerDrawing);
	G->SetViewPort();
}

ShadowMapping::~ShadowMapping()
{
}
