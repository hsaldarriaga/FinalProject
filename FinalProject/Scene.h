#pragma once

#include "Graphics.h"
#include "Ctrl360.h"
#include "MeshFactory.h"
#include "TerrainMesh.h"
#include "CompoundMesh.h"
#include "SimpleMesh.h"
#include "SunMesh.h"
#include "ShadowMapping.h"
#include "MotionBlurRender.h"
#include "Ocean.h"

class Scene
{
public:
	Scene(Graphics* g);
	bool Initialize();
	void Render();
	
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
	~Scene();
private:
	void RenderOnChange();
	bool InitShaders();
	bool CreateBuffers();
	bool LoadModels();
	bool SetParameters();

	bool ThereAreChanges;

	Graphics* G;
	Physx* PX;
	MeshFactory* Mfactory;
	CompoundMesh* Tree;
	TerrainMesh* Terrain;
	SunMesh* Sun;
	SimpleMesh* Rocks;
	Camera* Cmr;
	Ctrl360* Joystick;
	ShadowMapping* Shadows;
	MotionBlurRender* MotionBlur;
	Ocean* ocean;
	CComPtr<ID3D11PixelShader> PShader, PSSunShader;
	CComPtr<ID3D11VertexShader> VShader, VSSunShader;
	CComPtr<ID3D11InputLayout> InputLayout, SunInputLayout;
	CComPtr<ID3D11Buffer> cbMatrices;
	CComPtr<ID3D11Buffer> cbMaterial_Light;
	CComPtr<ID3D11Buffer> cbLight;
	CComPtr<ID3D11Buffer> cbEye;
	CComPtr<ID3D11SamplerState> SSTexture;

	DirectX::XMVECTOR LightDir;
	DirectX::XMVECTOR LightColor;
};

