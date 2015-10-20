#include "Scene.h"
#include "FileLoader.h"

Scene::Scene(Graphics* g)
{
	this->G = g;
	PX = new Physx();
	Mfactory = new MeshFactory(PX, G);
	ocean = new Ocean(G);
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
#pragma region Terrain Vertex Pixel
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/VertexShader.cso", &Data, &Size)))
		return false;
	D3D11_INPUT_ELEMENT_DESC desc[3];
	desc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	desc[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	desc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	if (FAILED(g->CreateInputLayout(desc, 3, Data, Size, &InputLayout.p))) {
		delete[] Data;
		return false;
	}
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VShader.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/PixelShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreatePixelShader(Data, Size, NULL, &PShader.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
#pragma endregion
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/SunVertexShader.cso", &Data, &Size)))
		return false;
	D3D11_INPUT_ELEMENT_DESC desc1[1];
	desc1[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (FAILED(g->CreateInputLayout(desc1, 1, Data, Size, &SunInputLayout.p))) {
		delete[] Data;
		return false;
	}
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VSSunShader.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/SunPixelShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreatePixelShader(Data, Size, NULL, &PSSunShader.p))) {
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
	ds.ByteWidth = sizeof(WORLD_VIEW_PROJ_BUFFER);
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.StructureByteStride = sizeof(WORLD_VIEW_PROJ_BUFFER);
	ds.Usage = D3D11_USAGE_DYNAMIC;
	hr = g->CreateBuffer(&ds, NULL, &cbMatrices);
	if (FAILED(hr)){ 
		return false; 
	}

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
	Sun = static_cast<SunMesh*>(Mfactory->Initialize(L"\\models\\sun.fbx", "Sun"));
	if (!Sun)
		return false;
	if (FAILED(ocean->Initialize()))
		return false;
	return true;
}
bool Scene::SetParameters()
{
	Cmr = new Camera(PX);
	MotionBlur = new MotionBlurRender(G);
	Cmr->SetCamera(DirectX::XMFLOAT3A(0.02f, 20.0f, 0.01f), DirectX::XMFLOAT3(-5.0f, 25.0f, 0.01f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	Cmr->SetParameters(DirectX::XMConvertToRadians(90.0f), (float)G->Width / (float)G->Height, 0.2f, 350.0f);
	LightColor = DirectX::XMLoadFloat4A(&DirectX::XMFLOAT4A(0.4f, 0.4f, 0.4f, 1.0f));
	DirectX::XMFLOAT2 SubFrustrums[] = { DirectX::XMFLOAT2(0.2f, 15.0f), DirectX::XMFLOAT2(15.0f, 30.0f), DirectX::XMFLOAT2(30.0f, 200.0f) };
	Shadows = new ShadowMapping(G, SubFrustrums, Cmr);
	if (!Shadows->Initialize())
		return false;
	Shadows->SetParameters(DirectX::XMFLOAT3(-200.0f, 200.0f, -1.0f), DirectX::XMFLOAT3(0.0f,0.0f,0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	LightDir = Shadows->getLighDir();
	Joystick = new Ctrl360(Cmr);
	Joystick->Init();
	DirectX::XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, Shadows->getLighPos());
	Sun->SetParams(&pos);
	MotionBlur->Initialize(Sun->Kd);
	return true;
}
void Scene::RenderOnChange() 
{
	
	Cmr->SizeChange((float)G->Width / (float)G->Height);
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE sub;
	d->Map(cbLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	LIGHT* buflight = reinterpret_cast<LIGHT*>(sub.pData);
	buflight->color = LightColor;
	buflight->dir = LightDir;
	d->Unmap(cbLight, 0);
	IMesh* meshes[1] = { Terrain };
	Shadows->Render(meshes, ARRAYSIZE(meshes));
	Shadows->AddBufferToShowShadows(Cmr);
	MotionBlur->UpdateBuffers(Cmr);
	ocean->SetLightsAndCameras(&LightDir, &Cmr->GetDir());
	ThereAreChanges = false;
	
}

void Scene::Render()
{
	ID3D11DeviceContext* d = G->pDevContext;
	if (!G->isReady())
	{
		ThereAreChanges = true;
		G->Ready(true);
	}
	d->IASetInputLayout(InputLayout);
	if (ThereAreChanges)
		RenderOnChange();
	Joystick->CaptureUserInteractions();
	PX->pPxScene->simulate(1.0f / 60.0f * 1000.0f);
	PX->pPxScene->fetchResults(true);
	Cmr->Update();
	Cmr->setView(); Cmr->SetProjection();

	D3D11_MAPPED_SUBRESOURCE sub;
	d->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	WORLD_VIEW_PROJ_BUFFER* data = reinterpret_cast<WORLD_VIEW_PROJ_BUFFER*>(sub.pData);
	data->World = DirectX::XMMatrixTranspose(*Terrain->getMatrix());
	data->View = DirectX::XMMatrixTranspose(Cmr->GetView());
	data->Proj = DirectX::XMMatrixTranspose(Cmr->GetProj());
	d->Unmap(cbMatrices, 0);
	//MotionBlur->Prepare();
	d->OMSetRenderTargets(1, &G->pRenderTargetView.p, G->pDepthStencilView);
	d->ClearRenderTargetView(G->pRenderTargetView, G->color);
	d->ClearDepthStencilView(G->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	d->Map(cbMaterial_Light, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	MATERIAL_LIGHT* material = reinterpret_cast<MATERIAL_LIGHT*>(sub.pData);
	material->Ka = Terrain->Ka;
	material->Kd = Terrain->Kd;
	d->Unmap(cbMaterial_Light, 0);

	d->IASetVertexBuffers(0, 1, &Terrain->VertexBuffer.p, &Terrain->stride, &Terrain->offset);
	d->IASetIndexBuffer(Terrain->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VShader, NULL, 0);
	d->VSSetConstantBuffers(0, 1, &cbMatrices.p);
	d->VSSetConstantBuffers(1, 1, &Shadows->cbLightMatrices.p);
	d->PSSetShader(PShader, NULL, 0);
	d->PSSetShaderResources(0, 1, &Terrain->resource.p);
	d->PSSetShaderResources(1, 1, &Shadows->shaderResource.p);
	d->PSSetSamplers(0, 1, &SSTexture.p);
	d->PSSetSamplers(1, 1, &Shadows->sampler.p);
	d->PSSetConstantBuffers(0, 1, &cbMaterial_Light.p);
	d->PSSetConstantBuffers(1, 1, &cbLight.p);
	d->PSSetConstantBuffers(2, 1, &Shadows->cbShadowParams.p);
	
	d->DrawIndexed(Terrain->getIndexesCount(), 0, 0);
	//Render Ocean
	ocean->Render(&(Cmr->GetView() * Cmr->GetProj()));

	//Render Sun
	MotionBlur->Prepare();
	d->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	data = reinterpret_cast<WORLD_VIEW_PROJ_BUFFER*>(sub.pData);
	data->World = DirectX::XMMatrixTranspose(*Sun->getMatrix());
	data->View = DirectX::XMMatrixTranspose(Cmr->GetView());
	data->Proj = DirectX::XMMatrixTranspose(Cmr->GetProj());
	d->Unmap(cbMatrices, 0);

	d->IASetInputLayout(SunInputLayout);
	d->IASetVertexBuffers(0, 1, &Sun->VertexBuffer.p, &Sun->stride, &Sun->offset);
	d->IASetIndexBuffer(Sun->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VSSunShader, NULL, 0);
	d->VSSetConstantBuffers(0, 1, &cbMatrices.p);

	d->PSSetShader(PSSunShader, NULL, 0);
	Sun->SetBuffers();
	d->DrawIndexed(Sun->getIndexesCount(), 0, 0);

	MotionBlur->Render();
	G->pSwapChain->Present(1, 0);
}

Scene::~Scene()
{
	if (Joystick)
		delete Joystick;
	if (Cmr)
		delete Cmr;
	if (ocean)
		delete ocean;
	if (MotionBlur)
		delete MotionBlur;
	if (Shadows)
		delete Shadows;
	if (Sun)
		delete Sun;
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
