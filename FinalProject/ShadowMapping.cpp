#include "ShadowMapping.h"
#include "Scene.h"
#include "FileLoader.h"

using namespace::DirectX;
//Este método consiste en dibujar toda la escena desde el punto de vista del sol y obtener el Depth Buffer
//Para renderizar el depth buffer es necesario cambiar la matriz de proyección por una ortogonal
ShadowMapping::ShadowMapping(Graphics* g, DirectX::XMFLOAT2 Zvalues[CASCADE_COUNT], Camera* Cmr)
{
	this->G = g;
	this->TextureSize = 2048;
	for (size_t i = 0; i < CASCADE_COUNT; i++)
	{
		//Se crea la misma Projección ortogonal pero con diferentes View Frustum,
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
	//Como no se va a pintar en pantalla, sino dentro de una textura, se asigna el tamaño de la textura al ViewPort
	ZeroMemory(&ViewPort, sizeof(D3D11_VIEWPORT));
	ViewPort.Height = (float)TextureSize;
	ViewPort.Width = (float)TextureSize;
	ViewPort.MinDepth = 0.0f;
	ViewPort.MaxDepth = 1.0f;
	return true;
}

bool ShadowMapping::CreateShadowBuffers()
{
	//Se crea un arreglo de texturas de tamaño CASCADE_COUNT(3)
	CComPtr<ID3D11Texture2D> textures;
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = CASCADE_COUNT;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.SampleDesc.Quality = 0;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	shadowMapDesc.Height = TextureSize;
	shadowMapDesc.Width = TextureSize;
	HRESULT hr = G->pDevice->CreateTexture2D(&shadowMapDesc, NULL, &textures.p);
	if (FAILED(hr))
		return false;
	//Por cada textura creada se le asigna un DepthStencilView para renderizar el depth buffer por cada ViewFrutum definido
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
	//ShaderResourceView necesario para poder leer el arreglo de texturas desde el sombreador
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
	//Sampler para leer el arreglo de texturas DepthBuffer, se agrega un filtro de tipo comparación
	//para implementar la técnica de Percentage Closer Filtering (PCF)
	D3D11_SAMPLER_DESC samplerdesc;
	ZeroMemory(&samplerdesc, sizeof(D3D11_SAMPLER_DESC));
	samplerdesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerdesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerdesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerdesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerdesc.MipLODBias = 0.0f;
	samplerdesc.ComparisonFunc = D3D11_COMPARISON_LESS;
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
	//Se define el Rasterize Stage para que no renderize la cara frontal de los modelos 3D para así disminuir
	//el error al mostrar la sombras
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
	//Se crea el buffer que va a contener la matriz de transformación de los modelos que se obtendrán su sombra
	hr = g->CreateBuffer(&ds, NULL, &cbMatrices);
	if (FAILED(hr)){ return false; }
	ds.ByteWidth = sizeof(XMFLOAT4);
	ds.StructureByteStride = sizeof(XMFLOAT4);
	hr = g->CreateBuffer(&ds, NULL, &cbShadowParams);
	if (FAILED(hr)){ return false; }
	ds.ByteWidth = sizeof(LIGHT_VIEW_PROJ_BUFFER);
	ds.StructureByteStride = sizeof(LIGHT_VIEW_PROJ_BUFFER);
	//Se crea el buffer que obtiene la matriz de transformación para renderizar desde el punto de vista del sol y obtener las sombras
	hr = g->CreateBuffer(&ds, NULL, &cbLightMatrices);
	if (FAILED(hr)){ return false; }
	return true;
}

void ShadowMapping::SetParameters(DirectX::XMFLOAT3 LightEye, DirectX::XMFLOAT3 LightTarget, DirectX::XMFLOAT3 LightUp)
{
	this->LightEye = XMLoadFloat3(&LightEye);
	this->LightTarget = XMLoadFloat3(&LightTarget);
	this->LightUp = XMLoadFloat3(&LightUp);
	View = XMMatrixLookAtLH(this->LightEye, this->LightTarget, this->LightUp);
}

DirectX::XMVECTOR ShadowMapping::getLighPos()
{
	return LightEye;
}

DirectX::XMVECTOR ShadowMapping::getLighDir()
{
	return XMVector3Normalize(LightTarget - LightEye);
}

void ShadowMapping::AddBufferToShowShadows()
{
	ID3D11DeviceContext* c = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE ss;
	c->Map(cbShadowParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	//Se definen los intervalos del View Frustum,a actualmente estos valores son arreglados debido
	//a un error de Blender al exportar los modelos al formato .fbx, no fue posible implementar Cascade Shadow Map
	//Por lo tanto se le asignaron estos valores para que siempre utilize la primera Textura del arreglo de Texturas del DepthBuffer
	//Haciendo que se implemente simplemente Shadow Mapping y no Cascade Shadow Mapping
	XMFLOAT3A vec = XMFLOAT3A(600, 800, 1000);
	//Podemos decir que la implementación está de CSM está hecha por no se está utilizando hasta poder arreglar el problema de los modelos.
	memcpy(ss.pData, &vec, sizeof(XMFLOAT3A));
	c->Unmap(cbShadowParams, 0);
	c->Map(cbLightMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &ss);
	LIGHT_VIEW_PROJ_BUFFER* ll = reinterpret_cast<LIGHT_VIEW_PROJ_BUFFER*>(ss.pData);
	ll->LightPos = LightEye;
	ll->LightView = DirectX::XMMatrixTranspose(View);
	//Debido al problema de exportación de los modelos con blender, Width y Height definidos en la matriz ortogonal
	//También son valores arreglados para poder mostrar la sombra del árbol correctamente
	ll->LightProj = XMMatrixTranspose(XMMatrixOrthographicLH((float)620, (float)620, 0.0f, 500.0f));
	c->Unmap(cbLightMatrices, 0);
}

void ShadowMapping::Render(IMesh* const* Meshes, UINT count)
{
	ID3D11DeviceContext* c = G->pDevContext;
	c->RSSetState(RasterizerShadow);
	c->RSSetViewports(1, &ViewPort);
	//Se asignan los sombreadores que renderizan las sombras con el Depth Buffer
	c->VSSetShader(VertexShader, NULL, 0);
	c->PSSetShader(PixelShader, NULL, 0);
	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	c->PSSetShaderResources(1, 1, pSRV);
	//Por cada View Frustum Definido se renderiza su correspondiente Depth Buffer
	for (size_t j = 0; j < CASCADE_COUNT; j++)
	{
		c->ClearDepthStencilView(shadowDepthStencilViews[j], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		c->OMSetRenderTargets(0, NULL, shadowDepthStencilViews[j]);
		//Por cada modelo que se insertó como párametro 'Meshes' se renderizará su sombra
		for (size_t i = 0; i < count; i++)
		{
			c->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			D3D11_MAPPED_SUBRESOURCE sub;
			if (Meshes[i]->MeshCount == 1) { //Si es un modelo 3D con un solo conjunto de vértices, como lo son las rocas
				c->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
				WORLD_VIEWxPROJ_BUFFER* data = reinterpret_cast<WORLD_VIEWxPROJ_BUFFER*>(sub.pData);
				data->World = DirectX::XMMatrixTranspose((*Meshes[i]->getMatrix()*XMMatrixScaling(5.0f, 5.0f, 5.0f)));
				data->ViewProj = DirectX::XMMatrixTranspose(View * Orthogonalm[j]);
				c->Unmap(cbMatrices, 0);
				c->VSSetConstantBuffers(0, 1, &cbMatrices.p);
				c->IASetVertexBuffers(0, 1, &Meshes[i]->VertexBuffer.p, &Meshes[i]->stride, &Meshes[i]->offset);
				c->IASetIndexBuffer(Meshes[i]->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
				c->DrawIndexed(Meshes[i]->getIndexesCount(), 0, 0);
			}
			else { //Si es un modelo 3D compuesto de varios conjuntos de vértices, como es el árbol, compuesto del tronco y las hojas.
				for (size_t k = 0; k < Meshes[i]->MeshCount; k++) {
					c->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
					WORLD_VIEWxPROJ_BUFFER* data = reinterpret_cast<WORLD_VIEWxPROJ_BUFFER*>(sub.pData);
					data->World = DirectX::XMMatrixTranspose(*Meshes[i]->getMatrix(k)*XMMatrixScaling(5.0f, 5.0f, 5.0f));
					data->ViewProj = DirectX::XMMatrixTranspose(View * Orthogonalm[j]);
					c->Unmap(cbMatrices, 0);
					c->VSSetConstantBuffers(0, 1, &cbMatrices.p);
					ID3D11Buffer* Vbuffs[1] = { Meshes[i]->getVertexBuffer(k) };
					UINT ST[1] = { Meshes[i]->getStride(k) }, OF[1] = { Meshes[i]->getOffset(k)};
					c->IASetVertexBuffers(0, 1, Vbuffs, ST, OF);
					c->IASetIndexBuffer(Meshes[i]->getIndexBuffer(k), DXGI_FORMAT_R32_UINT, 0);
					c->DrawIndexed(Meshes[i]->getIndexesCount(k), 0, 0);
				}
			}
			
		}
	}
	//Una vez terminado de renderizar en los Depth Buffers se configure el Graphic Pipeline para que vuelva a renderizar en pantalla
	G->pDevContext->OMSetRenderTargets(1, &G->pRenderTargetView.p, G->pDepthStencilView);
	c->RSSetState(RasterizerDrawing);
	//Ajusta el tamaño de la renderización al de la pantalla.
	G->SetViewPort();
}

ShadowMapping::~ShadowMapping()
{
}
