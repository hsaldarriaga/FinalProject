#pragma once
#include "ocean_simulator.h"
#include "Definitions.h"
//Esta clase sirve para utilizar los recursos obtenidos del código fuente de NVIDIA SDK11 SAMPLE OCEAN WATER SIMULATION
class Ocean
{
public:
	Ocean(Graphics* g);
	HRESULT Initialize();
	//Configura la dirección de la luz y el de la cámara para hacer el efecto de Specular Lighting
	void SetLightsAndCameras(DirectX::XMVECTOR* LightDir, DirectX::XMVECTOR* CmrDir);
	//Obtiene la matriz de transformación del terreno y su correspondiente HeightMap para definir la transparencia del agua.
	void SetHeightMapAndTerrainMatrices(ID3D11ShaderResourceView*, ID3D11Buffer*);
	//Se renderiza el oceano
	void Render(DirectX::XMMATRIX*);
	~Ocean();
#pragma region Allocator
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
#pragma endregion
private:

	HRESULT InitializeBuffers();
	HRESULT InitializeShaderResources();
	//Clase obtenida de NVIDIA SAMPLE SDK11 que contiene la parte matemática de la formación del agua
	OceanSimulator* simulator;
	Graphics* G;
	DirectX::XMMATRIX MLocal;
	UINT IndexCount;
	CComPtr<ID3D11InputLayout> InputLayout;
	CComPtr<ID3D11Buffer> VertexBuffer, IndexBuffer, MatricesBuffer, LightEyeBuffer, TerrainMatrices;
	CComPtr<ID3D11VertexShader> VSShader;
	CComPtr<ID3D11PixelShader> PSShader;
	//SamplerDisplacement el es el Sampler usado para obtener el desplasamiento de cada punto de los vértices que
	//conforman el agua y SamplerGradient es el sampler para obtener el vector normal de cada punto desplazado de los
	//vértices.
	CComPtr<ID3D11SamplerState> samplerDisplacement, SamplerGradient;
	//Añade transparencia al agua
	CComPtr<ID3D11BlendState> BlendWater;
	//Textura que tiene la alturas del terreno según la posicion (x,z) del vértice
	CComPtr<ID3D11ShaderResourceView> TerrainHeightMap;

	int iterator;
};

