#pragma once
#include "IMesh.h"
#include <atlbase.h>
#include <vector>
#include "Definitions.h"

class TerrainMesh : public IMesh
{
public:
	TerrainMesh(Graphics* g, Physx* px, LPCWSTR location);
	bool Initialize(FbxNode*);
	UINT getIndexCount();
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
	~TerrainMesh();
private:
	HRESULT Prepare();
	bool AddPhysics();
	std::vector<DataLayout> pDataforD3D;
	std::vector<UINT> pIndexforD3D;
public:
	
	CComPtr<ID3D11ShaderResourceView> resource;
	CComPtr<ID3D11Buffer> VertexBuffer;
	CComPtr<ID3D11Buffer> IndexBuffer;
	DirectX::XMVECTOR Ka; //ambient
	DirectX::XMVECTOR Ks; //specular
	DirectX::XMVECTOR Kd;
	float shininess;
};
