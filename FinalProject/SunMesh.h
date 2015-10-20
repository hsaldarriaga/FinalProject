#pragma once
#include "IMesh.h"
#include <atlbase.h>
#include <vector>
#include "Definitions.h"

class SunMesh : public IMesh
{
public:
	SunMesh(Graphics*, Physx*, LPCWSTR);
	bool Initialize(FbxNode*);
	void SetBuffers();
	void SetParams(DirectX::XMFLOAT3* position);
	int getIndexesCount();
	DirectX::XMMATRIX* getMatrix();
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
	~SunMesh();
	DirectX::XMVECTOR Kd;
private:
	HRESULT Prepare();
	physx::PxMat44 World;
	std::vector<DirectX::XMVECTOR> pDataforD3D;
	std::vector<UINT> pIndexforD3D;


	CComPtr<ID3D11Buffer> cbSun_Params;

	__declspec(align(16)) struct SUN_PARAMS
	{
		DirectX::XMVECTOR Kd; // more blue
	};
};

