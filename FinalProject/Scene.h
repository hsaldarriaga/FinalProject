#pragma once

#include "Graphics.h"
#include "Ctrl360.h"
#include "MeshFactory.h"
#include "TerrainMesh.h"

class Scene
{
public:
	Scene(Graphics* g);
	bool Initialize();
	void Render();
	
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
	~Scene();

private:
	void RenderOnChange();
	bool InitShaders();
	bool CreateBuffers();
	bool LoadModels();
	bool SetParameters();

	bool ThereAreChanges = true;

	Graphics* G;
	Physx* PX = NULL;
	MeshFactory* Mfactory = NULL;
	TerrainMesh* Terrain = NULL;
	Camera* Cmr = NULL;
	Ctrl360* Joystick = NULL;

	CComPtr<ID3D11PixelShader> PShader;
	CComPtr<ID3D11VertexShader> VShader;
	CComPtr<ID3D11InputLayout> InputLayout;
	CComPtr<ID3D11Buffer> cbMatrices;
	CComPtr<ID3D11Buffer> cbMaterial_Light;
	CComPtr<ID3D11Buffer> cbLight;
	CComPtr<ID3D11Buffer> cbEye;
	CComPtr<ID3D11SamplerState> SSTexture;
	D3D11_INPUT_ELEMENT_DESC desc[3];

	DirectX::XMVECTOR LightDir;
	DirectX::XMVECTOR LightColor;
};

