#include "Ocean.h"
#include "FileLoader.h"
using namespace::DirectX;

Ocean::Ocean(Graphics* g)
{
	this->G = g;
	MLocal = XMMatrixIdentity();
	OceanParameter ocean_param;
	// The size of displacement map. In this sample, it's fixed to 512.
	ocean_param.dmap_dim = 512;
	// The side length (world space) of square patch
	ocean_param.patch_length = 2000.0f;
	// Adjust this parameter to control the simulation speed
	ocean_param.time_scale = 0.8f;
	// A scale to control the amplitude. Not the world space height
	ocean_param.wave_amplitude = 0.35f;
	// 2D wind direction. No need to be normalized
	ocean_param.wind_dir = XMFLOAT2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	ocean_param.wind_speed = 600.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	ocean_param.wind_dependency = 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	// pointy crests.
	ocean_param.choppy_scale = 1.3f;
	simulator = new OceanSimulator(ocean_param, G->pDevice);
	simulator->updateDisplacementMap(0.0f);
}

HRESULT Ocean::Initialize()
{
	HRESULT hr = InitializeBuffers();
	if (FAILED(hr)) { return hr; }
	hr = InitializeShaderResources();
	return hr;
}

HRESULT Ocean::InitializeBuffers()
{
	HRESULT hr;
	int num_verts = (256 + 1) * (256 + 1);
	XMFLOAT2* pV = new XMFLOAT2[num_verts];

	int i, j;
	for (i = 0; i <= 256; i++)
	{
		for (j = 0; j <= 256; j++)
		{
			XMFLOAT2 vec;
			vec.x = (float)(i - 128);
			vec.y = (float)(j - 128);
			pV[i * (256 + 1) + j] = vec;
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
	int num_ind = 256 * 256 * 6;
	UINT* indexes = new UINT[num_ind];
	UINT r = 0;
	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
		{
			indexes[r] = i * (256 + 1) + j; r++;
			indexes[r] = (i + 1) * (256 + 1) + (j + 1); r++;
			indexes[r] = (i + 1) * (256 + 1) + j; r++;

			indexes[r] = i * (256 + 1) + j; r++;
			indexes[r] = i * (256 + 1) + (j + 1); r++;
			indexes[r] = (i + 1) * (256 + 1) + (j + 1); r++;
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
	if (FAILED(g->CreateInputLayout(desc, 3, Data, Size, &InputLayout.p))) {
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

	hr = G->pDevice->CreateSamplerState(&sam_desc, &samplerDisplacement);
	if (FAILED(hr)) { return hr; }
	sam_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sam_desc.MaxAnisotropy = 8;
	hr = G->pDevice->CreateSamplerState(&sam_desc, &SamplerGradient);
	if (FAILED(hr)) { return hr; }
	return hr;
}

void Ocean::SetLightsAndCameras(XMVECTOR* LightDir, XMVECTOR* CmrDir)
{
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE subresource;
	d->Map(MatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	LIGHTDIR_EYEDIR* pMatrices = reinterpret_cast<LIGHTDIR_EYEDIR*>(subresource.pData);
	pMatrices->light_dir = *LightDir;
	pMatrices->eye_dir = *CmrDir;
	d->Unmap(MatricesBuffer, 0);
}

void Ocean::Render(XMMATRIX* MatrixViewProj)
{
	simulator->updateDisplacementMap(1 / 60 * 1000);
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE subresource;
	d->Map(MatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	WORLD_VIEWxPROJ_BUFFER* pMatrices = reinterpret_cast<WORLD_VIEWxPROJ_BUFFER*>(subresource.pData);
	pMatrices->ViewProj = *MatrixViewProj;
	pMatrices->World = MLocal;
	d->Unmap(MatricesBuffer, 0);

	d->IASetInputLayout(InputLayout);
	UINT Stride = sizeof(XMFLOAT2), Offset = 0;
	d->IASetVertexBuffers(0, 1, &VertexBuffer.p, &Stride, &Offset);
	d->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d->VSSetShader(VSShader, NULL, 0);
	ID3D11ShaderResourceView* vsresources[1] = { simulator->getD3D11DisplacementMap() };
	d->VSSetShaderResources(0, 1, vsresources);
	ID3D11SamplerState* vssamplers[1] = { samplerDisplacement };
	d->VSSetSamplers(0, 1, vssamplers);
	ID3D11Buffer* VSBuffers[1] = { MatricesBuffer };
	d->VSSetConstantBuffers(0, 1, VSBuffers);

	d->PSSetShader(PSShader, NULL, 0);
	ID3D11ShaderResourceView* psresources[1] = { simulator->getD3D11GradientMap() };
	d->PSSetShaderResources(0, 1, psresources);
	ID3D11SamplerState* pssamplers[1] = { SamplerGradient };
	d->PSSetSamplers(0, 1, pssamplers);
	ID3D11Buffer* PSBuffers[1] = { LightEyeBuffer };
	d->PSSetConstantBuffers(0, 1, PSBuffers);

	d->DrawIndexed(IndexCount, 0, 0);
}

Ocean::~Ocean()
{
	if (simulator)
		delete simulator;
}
