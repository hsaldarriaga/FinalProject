#include "Scene.h"
#include "FileLoader.h"
#include "ToTexture.h"
Scene::Scene(Graphics* g)
{
	this->G = g;
	PX = new Physx();
	Mfactory = new MeshFactory(PX, G);
	ocean = new Ocean(G);
	Terrain = NULL;
	Rocks = NULL;
	Sun = NULL;
	Cmr = NULL;
	Joystick = NULL;
	Shadows = NULL;
	MotionBlur = NULL;
	ThereAreChanges = true;
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
	D3D11_INPUT_ELEMENT_DESC desc[3] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

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
	D3D11_INPUT_ELEMENT_DESC desc1[] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
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
	//Se obtiene el modelo 3D del terreno
	Terrain = static_cast<TerrainMesh*>(Mfactory->Initialize(L"\\models\\beach.fbx", "Terrain"));
	if (!Terrain)
		return false;
	//Se añade su actor al mundo físico definido por PxScene
	PX->pPxScene->addActor(*Terrain->actor);
	//Se crea el sol
	Sun = static_cast<SunMesh*>(Mfactory->Initialize(L"\\models\\sun.fbx", "Sun"));
	if (!Sun)
		return false;
	//Se crea el árbol compuesto por dos conjuntos de vértices, el tronco y las hojas
	Tree = static_cast<CompoundMesh*>(Mfactory->Initialize(L"\\models\\tree.fbx", "Compound"));
	PX->pPxScene->addActors(Tree->actors, Tree->MeshCount);
	if (!Tree)
		return false;
	//Se obtienen las rocas
	Rocks = static_cast<SimpleMesh*>(Mfactory->Initialize(L"\\models\\rocks.fbx", "Simple"));
	if (!Rocks)
		return false;
	PX->pPxScene->addActor(*Rocks->actor);
	if (FAILED(ocean->Initialize()))
		return false;
	return true;
}
bool Scene::SetParameters()
{
	Cmr = new Camera(PX);
	MotionBlur = new MotionBlurRender(G);
	//Parámetros de la View Matrix
	Cmr->SetCamera(DirectX::XMFLOAT3A(3.0f, 20.0f, 3.0f), DirectX::XMFLOAT3(3.0f, 20.0f, 5.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	//Parámetros de la Pespective Projection Matrix
	Cmr->SetParameters(DirectX::XMConvertToRadians(90.0f), (float)G->Width / (float)G->Height, 0.2f, 350.0f);
	LightColor = DirectX::XMLoadFloat4A(&DirectX::XMFLOAT4A(0.4f, 0.4f, 0.4f, 1.0f));
	//Las distancias de que hay entre cada View Frustum
	DirectX::XMFLOAT2 SubFrustrums[] = { DirectX::XMFLOAT2(0.0f, 400.0f), DirectX::XMFLOAT2(100.0f, 200.0f), DirectX::XMFLOAT2(200.0f, 400.0f) };
	Shadows = new ShadowMapping(G, SubFrustrums, Cmr);
	if (!Shadows->Initialize())
		return false;
	//Parámetros como la posición del sol, hacía donde ilumina el sol y el vector 'Arriba'(0,1,0)
	Shadows->SetParameters(DirectX::XMFLOAT3(-200.0f, 300.0f, -1.0f), DirectX::XMFLOAT3(0.0f,0.0f,0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
	LightDir = Shadows->getLighDir();
	//Se inicializa el control de Xbox 360
	Joystick = new Ctrl360(Cmr);
	Joystick->Init();
	DirectX::XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, Shadows->getLighPos());
	Sun->SetParams(&pos);
	//Se inicializa MotionBlur obteniendo el color del sol, para poder aplicar el color al hacer blur al sol.
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
	//Solo pinta la sombra del árbol
	IMesh* meshes[1] = { Tree};
	//Se recalcula las sombras cuando hay algun cambio en el tamaño de la pantalla
	Shadows->Render(meshes, ARRAYSIZE(meshes));
	Shadows->AddBufferToShowShadows();
	MotionBlur->UpdateBuffers(Cmr);
	ocean->SetLightsAndCameras(&LightDir, &Cmr->GetDir());
	ocean->SetHeightMapAndTerrainMatrices(Terrain->getHeightMap(), cbMatrices);
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
	//Se define la estructura de los vértices que entran el la Graphics Pipeline
	d->IASetInputLayout(InputLayout);
	if (ThereAreChanges)
		RenderOnChange();
	//Se capturan las interacciones del control que intervienen con el movimiento de la cámara
	Joystick->CaptureUserInteractions();
	//Se simula el mundo físico, para detectar colisiones y movimientos
	PX->pPxScene->simulate(1.0f / 60.0f * 1000.0f);
	//Hasta que no termine de calcular la posición de los objetos en el mundo físico bloquea este hilo en este punto.
	PX->pPxScene->fetchResults(true);
	//Actualiza las matrices de la cámara por si se hizo algun movimiento con el control.
	Cmr->Update();
	Cmr->setView(); Cmr->SetProjection();
	//Se establece el Back Buffer y el DepthBuffer
	d->ClearRenderTargetView(G->pRenderTargetView, G->color);
	d->ClearDepthStencilView(G->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	/*
	d->OMSetRenderTargets(1, &G->pRenderTargetView.p, G->pDepthStencilView);
	ToTexture* tt = new ToTexture(G);
	tt->Initialize();
	tt->SizeChanged(2048, 2048);
	tt->RenderResourceToScreen(Shadows->shaderResource.p);
	delete tt;*/

	//Todo lo que se renderize de aquí hasta que se reasignen el BackBuffer y Depth Buffer, se le hará la técnica de Gaussian Blur
	MotionBlur->Prepare();
	D3D11_MAPPED_SUBRESOURCE sub;
	d->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	WORLD_VIEW_PROJ_BUFFER* data = reinterpret_cast<WORLD_VIEW_PROJ_BUFFER*>(sub.pData);
	//Se asignan las matrices del sol y la cámara
	data->World = DirectX::XMMatrixTranspose(*Sun->getMatrix());
	data->View = DirectX::XMMatrixTranspose(Cmr->GetView());
	data->Proj = DirectX::XMMatrixTranspose(Cmr->GetProj());
	d->Unmap(cbMatrices, 0);

	d->IASetInputLayout(SunInputLayout);
	//Se asignan los vértices del sol y los índices
	d->IASetVertexBuffers(0, 1, &Sun->VertexBuffer.p, &Sun->stride, &Sun->offset);
	d->IASetIndexBuffer(Sun->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Se asigna las matrices de transformación y el sombreador de vértices(VertexShader)
	d->VSSetShader(VSSunShader, NULL, 0);
	d->VSSetConstantBuffers(0, 1, &cbMatrices.p);

	d->PSSetShader(PSSunShader, NULL, 0);
	Sun->SetBuffers();
	//Se renderiza el sol
	d->DrawIndexed(Sun->getIndexesCount(), 0, 0);
	//Se aplica Blur lo que se ha renderizado hasta ahora (Sol) y lo dibuja en pantalla
	MotionBlur->Render();
	//Se desactiva blur y se dibuja normalmente en pantalla
	d->OMSetRenderTargets(1, &G->pRenderTargetView.p, G->pDepthStencilView);

	//Draw Tree
	Tree->Render(cbMatrices.p, SSTexture.p, cbLight.p, &Cmr->GetView(), &Cmr->GetProj());
	//Draw Rocks
	Rocks->Render(cbMatrices.p, SSTexture.p, cbLight.p, &Cmr->GetView(), &Cmr->GetProj());
	//Start Render Terrain
	d->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	data = reinterpret_cast<WORLD_VIEW_PROJ_BUFFER*>(sub.pData);
	data->World = DirectX::XMMatrixTranspose(*Terrain->getMatrix());
	data->View = DirectX::XMMatrixTranspose(Cmr->GetView());
	data->Proj = DirectX::XMMatrixTranspose(Cmr->GetProj());
	d->Unmap(cbMatrices, 0);

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
	//Render Ocean Inmediately after Terrain
	ocean->Render(&(Cmr->GetView() * Cmr->GetProj()));

	G->pSwapChain->Present(1, 0);
}

Scene::~Scene()
{
	if (Joystick)
		delete Joystick;
	if (Cmr)
		delete Cmr;
	if (Tree)
		delete Tree;
	if (ocean)
		delete ocean;
	if (MotionBlur)
		delete MotionBlur;
	if (Shadows)
		delete Shadows;
	if (Rocks)
		delete Rocks;
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
