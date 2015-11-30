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
	int getIndexesCount();
	DirectX::XMMATRIX* getMatrix();
	ID3D11ShaderResourceView* getHeightMap();
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
	~TerrainMesh();
private:
	HRESULT Prepare();
	bool AddPhysics();
	std::vector<DataLayout> pDataforD3D;
	std::vector<UINT> pIndexforD3D;
public:
	
	CComPtr<ID3D11ShaderResourceView> resource;
	CComPtr<ID3D11ShaderResourceView> HeightMap;
	DirectX::XMVECTOR Ka; //ambient
	DirectX::XMVECTOR Kd;
	DirectX::XMMATRIX World;
};

