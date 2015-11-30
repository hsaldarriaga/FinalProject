#include "Ocean.h"
#include "FileLoader.h"
using namespace::DirectX;

Ocean::Ocean(Graphics* g)
{
	this->G = g;
	MLocal = XMMatrixIdentity();
	//La implementación del modelo matemático de las olas fue tomado de NVIDIA SDK11 Sample
	OceanParameter ocean_param;
	// The size of displacement map. In this sample, it's fixed to 512.
	ocean_param.dmap_dim = 512;
	// The side length (world space) of square patch
	ocean_param.patch_length = 512.0f;
	// Adjust this parameter to control the simulation speed
	ocean_param.time_scale = 1.0f;
	// A scale to control the amplitude. Not the world space height
	ocean_param.wave_amplitude = 1.5f;
	// 2D wind direction. No need to be normalized
	ocean_param.wind_dir = XMFLOAT2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	ocean_param.wind_speed = 200.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	ocean_param.wind_dependency = 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	// pointy crests.
	ocean_param.choppy_scale = 2.0f;
	simulator = new OceanSimulator(ocean_param, G->pDevice);
	simulator->updateDisplacementMap(0.0f);
	iterator = 0;
}

HRESULT Ocean::Initialize()
{
	HRESULT hr = InitializeBuffers();
	if (FAILED(hr)) { return hr; }
	hr = InitializeShaderResources();
	return hr;
}

void Ocean::SetHeightMapAndTerrainMatrices(ID3D11ShaderResourceView* HeightMap, ID3D11Buffer* Terrain)
{
	this->TerrainHeightMap = HeightMap;
	this->TerrainMatrices = Terrain;
}

HRESULT Ocean::InitializeBuffers()
{
	HRESULT hr;
	int num_verts = (512 + 1) * (512 + 1);
	XMFLOAT2* pV = new XMFLOAT2[num_verts];

	int i, j;
	//Esta función crea un gran plano rectangular, formado por triángulos, y compuesto por 513 x 513 vértices.
	for (i = 0; i <= 512; i++)
	{
		for (j = 0; j <= 512; j++)
		{
			XMFLOAT2 vec;
			vec.x = (float)(i - 256);
			vec.y = (float)(j - 256);
			pV[i * (512 + 1) + j] = vec;
		}
	}
	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = num_verts * sizeof(XMFLOAT2);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;
	vb_desc.StructureByteStride = sizeof(XMFLOAT2);

	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = pV;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;
	hr = G->pDevice->CreateBuffer(&vb_desc, &init_data, &VertexBuffer.p);
	delete[] pV;
	if (FAILED(hr)) { return hr; }
	/*
	First Triangel
		left, top
		right, bottom
		left, bottom
	Second Triangle
		left, top
		right, top
		right, bottom
	*/
	int num_ind = 512 * 512 * 6;
	UINT* indexes = new UINT[num_ind];
	UINT r = 0;
	//Los índices que determinan como se pintará cada triángulo del plano.
	for (i = 0; i < 512; i++)
	{
		for (j = 0; j < 512; j++)
		{
			indexes[r] = i * (512 + 1) + j; r++;
			indexes[r] = (i + 1) * (512 + 1) + (j + 1); r++;
			indexes[r] = (i + 1) * (512 + 1) + j; r++;

			indexes[r] = i * (512 + 1) + j; r++;
			indexes[r] = i * (512 + 1) + (j + 1); r++;
			indexes[r] = (i + 1) * (512 + 1) + (j + 1); r++;
		}
	}
	IndexCount = num_ind;
	vb_desc.ByteWidth = num_ind * sizeof(UINT);
	vb_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vb_desc.StructureByteStride = sizeof(UINT);
	init_data.pSysMem = indexes;
	hr = G->pDevice->CreateBuffer(&vb_desc, &init_data, &IndexBuffer.p);
	delete[] indexes;
	if (FAILED(hr)) { return hr; }
	D3D11_BUFFER_DESC buffer_desc;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.ByteWidth = sizeof(WORLD_VIEWxPROJ_BUFFER);
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = sizeof(WORLD_VIEWxPROJ_BUFFER);
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	hr = G->pDevice->CreateBuffer(&buffer_desc, NULL, &MatricesBuffer.p);
	if (FAILED(hr)) { return hr; }
	buffer_desc.ByteWidth = sizeof(LIGHTDIR_EYEDIR);
	buffer_desc.StructureByteStride = sizeof(LIGHTDIR_EYEDIR);
	hr = G->pDevice->CreateBuffer(&buffer_desc, NULL, &LightEyeBuffer.p);
	return hr;
}

HRESULT Ocean::InitializeShaderResources()
{
	HRESULT hr;
	ID3D11Device* g = G->pDevice;
	BYTE* Data;
	LONG Size;
	//Para renderizar el agua, sólo es necesario la posición, el color se determina con ayuda del HeightMap del terreno.
	D3D11_INPUT_ELEMENT_DESC desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/OceanVertexShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VSShader.p))) {
		delete[] Data;
		return false;
	}
	if (FAILED(g->CreateInputLayout(desc, 1, Data, Size, &InputLayout.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/OceanPixelShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreatePixelShader(Data, Size, NULL, &PSShader.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;

	D3D11_SAMPLER_DESC sam_desc;
	sam_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.MipLODBias = 0;
	sam_desc.MaxAnisotropy = 1;
	sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sam_desc.BorderColor[0] = 1.0f;
	sam_desc.BorderColor[1] = 1.0f;
	sam_desc.BorderColor[2] = 1.0f;
	sam_desc.BorderColor[3] = 1.0f;
	sam_desc.MinLOD = 0;
	sam_desc.MaxLOD = FLT_MAX;

	hr = g->CreateSamplerState(&sam_desc, &samplerDisplacement);
	if (FAILED(hr)) { return hr; }
	sam_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sam_desc.MaxAnisotropy = 8;
	hr = g->CreateSamplerState(&sam_desc, &SamplerGradient);
	if (FAILED(hr)) { return hr; }
	D3D11_BLEND_DESC bdesc;
	ZeroMemory(&bdesc, sizeof(D3D11_BLEND_DESC));
	bdesc.AlphaToCoverageEnable = FALSE;
	bdesc.IndependentBlendEnable = FALSE;
	bdesc.RenderTarget[0].BlendEnable = TRUE;
	bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
	bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	bdesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	bdesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	bdesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//Se establece un RenderState para que el agua pueda ser transparente
	hr = g->CreateBlendState(&bdesc, &BlendWater.p);
	return hr;
}

void Ocean::SetLightsAndCameras(XMVECTOR* LightDir, XMVECTOR* CmrDir)
{
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE subresource;
	d->Map(LightEyeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	//Seguarda la dirección de la vista de la cámara y la luz para hacer los cálculos del Specular Light
	LIGHTDIR_EYEDIR* pMatrices = reinterpret_cast<LIGHTDIR_EYEDIR*>(subresource.pData);
	pMatrices->light_dir = *LightDir;
	pMatrices->eye_dir = *CmrDir;
	d->Unmap(LightEyeBuffer, 0);
}

void Ocean::Render(XMMATRIX* MatrixViewProj)
{
	//Se actualiza el modelo matemática de las olas, con ayuda de la FFT se mueven las olas un instante de tiempo
	simulator->updateDisplacementMap(iterator * 0.0002f * (1.0f / 60.0f * 1000.0f));
	iterator++;
	if (iterator >= 100000000)
		iterator = 0;
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE subresource;
	d->Map(MatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	WORLD_VIEWxPROJ_BUFFER* pMatrices = reinterpret_cast<WORLD_VIEWxPROJ_BUFFER*>(subresource.pData);
	pMatrices->ViewProj = DirectX::XMMatrixTranspose(*MatrixViewProj);
	pMatrices->World = DirectX::XMMatrixTranspose(MLocal);
	d->Unmap(MatricesBuffer, 0);

	d->IASetInputLayout(InputLayout);
	UINT Stride = sizeof(XMFLOAT2), Offset = 0;
	d->IASetVertexBuffers(0, 1, &VertexBuffer.p, &Stride, &Offset);
	d->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VSShader, NULL, 0);
	ID3D11ShaderResourceView* vsresources[2] = { simulator->getD3D11DisplacementMap(), TerrainHeightMap};
	d->VSSetShaderResources(0, 2, vsresources);
	ID3D11SamplerState* vssamplers[1] = { samplerDisplacement};
	d->VSSetSamplers(0, 1, vssamplers);
	ID3D11Buffer* VSBuffers[2] = { MatricesBuffer, TerrainMatrices };
	d->VSSetConstantBuffers(0, 2, VSBuffers);

	d->PSSetShader(PSShader, NULL, 0);
	ID3D11ShaderResourceView* psresources[1] = { simulator->getD3D11GradientMap()};
	d->PSSetShaderResources(0, 1, psresources);
	ID3D11SamplerState* pssamplers[1] = { SamplerGradient};
	d->PSSetSamplers(0, 1, pssamplers);
	ID3D11Buffer* PSBuffers[1] = { LightEyeBuffer };
	d->PSSetConstantBuffers(0, 1, PSBuffers);

	d->OMSetBlendState(BlendWater, NULL, 0xffffffff);
	d->DrawIndexed(IndexCount, 0, 0);

	d->OMSetBlendState(NULL, NULL, 0xffffffff);
	vsresources[0] = NULL;
	d->VSSetShaderResources(0, 1, vsresources);
	psresources[0] = NULL;
	d->PSSetShaderResources(0, 1, psresources);
}

Ocean::~Ocean()
{
	if (simulator)
		delete simulator;
}
