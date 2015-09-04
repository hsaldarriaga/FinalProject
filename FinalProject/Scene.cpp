#include "Scene.h"
#include "FileLoader.h"

Scene::Scene(Graphics* g)
{
	this->G = g;
	PX = new Physx();
	Mfactory = new MeshFactory(PX, G);
	desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	desc[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	desc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 };
}

bool Scene::Initialize()
{
	if (!PX->Init((void **)&G->pDevice.p))
		return false;
	if (!InitShaders())
		return false;
	if (!CreateBuffers())
		return false;
	if (!LoadModels())
		return false;
	if (!SetParameters())
		return false;
	return true;
}

bool Scene::InitShaders()
{
	ID3D11Device* g = G->pDevice;
	BYTE* Data;
	LONG Size;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/VertexShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreateInputLayout(desc, 3, Data, Size, &InputLayout))) {
		delete[] Data;
		return false;
	}
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VShader))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/PixelShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreatePixelShader(Data, Size, NULL, &PShader))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	return true;
}
bool Scene::CreateBuffers()
{
	ID3D11Device* g = G->pDevice;
	HRESULT hr;
	D3D11_BUFFER_DESC ds;
	ZeroMemory(&ds, sizeof(D3D11_BUFFER_DESC));
	ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ds.ByteWidth = sizeof(VS_CONSTANT);
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.StructureByteStride = sizeof(VS_CONSTANT);
	ds.Usage = D3D11_USAGE_DYNAMIC;
	hr = g->CreateBuffer(&ds, NULL, &cbMatrices);
	if (FAILED(hr)){ return false; }

	ZeroMemory(&ds, sizeof(D3D11_BUFFER_DESC));
	ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ds.ByteWidth = sizeof(MATERIAL_LIGHT);
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.StructureByteStride = sizeof(MATERIAL_LIGHT);
	ds.Usage = D3D11_USAGE_DYNAMIC;
	hr = g->CreateBuffer(&ds, NULL, &cbMaterial_Light);
	if (FAILED(hr)){ return false; }

	ZeroMemory(&ds, sizeof(D3D11_BUFFER_DESC));
	ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ds.ByteWidth = sizeof(LIGHT);
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.StructureByteStride = sizeof(LIGHT);
	ds.Usage = D3D11_USAGE_DYNAMIC;
	hr = g->CreateBuffer(&ds, NULL, &cbLight);
	if (FAILED(hr)){ return false; }

	ZeroMemory(&ds, sizeof(D3D11_BUFFER_DESC));
	ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ds.ByteWidth = sizeof(EYE);
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.StructureByteStride = sizeof(EYE);
	ds.Usage = D3D11_USAGE_DYNAMIC;
	hr = g->CreateBuffer(&ds, NULL, &cbEye);
	if (FAILED(hr)){ return false; }

	D3D11_SAMPLER_DESC dc;
	dc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	dc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	dc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	dc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	dc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	dc.MaxAnisotropy = 16;
	dc.MaxLOD = D3D11_FLOAT32_MAX;
	dc.MinLOD = 0;
	dc.MipLODBias = 0;
	hr = g->CreateSamplerState(&dc, &SSTexture);
	if (FAILED(hr)){ return false; }
	return true;
}
bool Scene::LoadModels()
{
	Terrain = static_cast<TerrainMesh*>(Mfactory->Initialize(L"\\models\\beach.fbx", "Terrain"));
	if (!Terrain)
		return false;
	PX->pPxScene->addActor(*Terrain->actor);
	return true;
}
bool Scene::SetParameters()
{
	Cmr = new Camera(PX);
	Cmr->SetCamera(DirectX::XMFLOAT3A(1.0f, 5.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 5.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	Cmr->SetParameters(DirectX::XMConvertToRadians(90.0f), (float)G->Width / (float)G->Height, 0.2f, 200.0f);
	LightDir = DirectX::XMLoadFloat3A(&DirectX::XMFLOAT3A(-5.0f, 3.0f, 0.0f));
	LightColor = DirectX::XMLoadFloat4A(&DirectX::XMFLOAT4A(0.4f, 0.4f, 0.4f, 1.0f));

	Joystick = new Ctrl360(Cmr);
	Joystick->Init();
	return true;
}

void Scene::RenderOnChange() 
{
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE sub;
	d->Map(cbLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	LIGHT* buflight = reinterpret_cast<LIGHT*>(sub.pData);
	buflight->color = LightColor;
	buflight->dir = LightDir;
	d->Unmap(cbLight, 0);
	ThereAreChanges = false;
}

void Scene::Render()
{
	if (ThereAreChanges)
		RenderOnChange();
	Joystick->CaptureUserInteractions();
	PX->pPxScene->simulate(1 / 60 * 1000);
	PX->pPxScene->fetchResults(true);
	ID3D11DeviceContext* d = G->pDevContext;
	Cmr->Update();
	Cmr->setView(); Cmr->SetProjection();
	D3D11_MAPPED_SUBRESOURCE sub;
	d->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	VS_CONSTANT* data = reinterpret_cast<VS_CONSTANT*>(sub.pData);
	data->World = DirectX::XMMatrixTranspose(*Terrain->getMatrix());
	data->ViewProj = Cmr->GetViewProj();
	d->Unmap(cbMatrices, 0);
	d->Map(cbMaterial_Light, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	MATERIAL_LIGHT* material = reinterpret_cast<MATERIAL_LIGHT*>(sub.pData);
	material->Ka = Terrain->Ka;
	material->Kd = Terrain->Kd;
	material->Ks = Terrain->Ks;
	material->shininess = Terrain->shininess;
	d->Unmap(cbMaterial_Light, 0);
	float color[] = { 0.3294f, 0.4627f, 0.7490f, 1.0f };
	d->ClearRenderTargetView(G->pRenderTargetView, color);
	d->ClearDepthStencilView(G->pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);
	d->IASetInputLayout(InputLayout);
	UINT Stride = sizeof(DataLayout);
	UINT Offset = 0;
	d->IASetVertexBuffers(0, 1, &Terrain->VertexBuffer.p, &Stride, &Offset);
	d->IASetIndexBuffer(Terrain->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VShader, NULL, 0);
	d->VSSetConstantBuffers(0, 1, &cbMatrices.p);
	d->PSSetShader(PShader, NULL, 0);
	d->PSSetShaderResources(0, 1, &Terrain->resource.p);
	d->PSSetSamplers(0, 1, &SSTexture.p);
	d->PSSetConstantBuffers(0, 1, &cbEye.p);
	d->PSSetConstantBuffers(1, 1, &cbMaterial_Light.p);
	d->PSSetConstantBuffers(2, 1, &cbLight.p);
	d->DrawIndexed(Terrain->getIndexCount(), 0, 0);

	G->pSwapChain->Present(1, 0);
}

Scene::~Scene()
{
	if (Joystick)
		delete Joystick;
	if (Cmr)
		delete Cmr;
	if (Terrain)
		delete Terrain;
	if (Mfactory)
		delete Mfactory;
	if (PX)
		delete PX;
	if (G) {
		G->Release();
		delete G;
	}
}
