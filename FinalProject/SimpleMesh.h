#pragma once
#include "IMesh.h"
#include <atlbase.h>
#include <vector>
#include "Definitions.h"

class SimpleMesh : public IMesh
{
public:
	SimpleMesh(Graphics* g, Physx* px, LPCWSTR location);
	bool Initialize(FbxNode*);
	int getIndexesCount();
	DirectX::XMMATRIX* getMatrix();
	void Render(ID3D11Buffer* cbMatrices, ID3D11SamplerState* SSTexture, ID3D11Buffer* cbLight, DirectX::XMMATRIX* View, DirectX::XMMATRIX* Proj);
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
	~SimpleMesh();
private:
	HRESULT Prepare();
	bool AddPhysics();
	HRESULT InitializeShaderResources();
	std::vector<DataLayout> pDataforD3D;
	std::vector<UINT> pIndexforD3D;
	CComPtr<ID3D11VertexShader> VSShader;
	CComPtr<ID3D11PixelShader> PSShader;
	CComPtr<ID3D11InputLayout> InputLayout;
public:

	CComPtr<ID3D11ShaderResourceView> resource;
	DirectX::XMMATRIX World;
};

