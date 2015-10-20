#pragma once
#include <Windows.h>
#include "Graphics.h"
#include "Physx.h"
#include <fbxsdk.h>
#include <DirectXMath.h>

#pragma comment(lib, "libfbxsdk-mt.lib")

class IMesh
{
public:
	IMesh(Graphics* g, Physx* px, LPCWSTR location) { this->G = g; this->PX = px; this->location = location;  }
	virtual bool Initialize(FbxNode*) = 0;
	virtual int getIndexesCount() = 0;
	virtual DirectX::XMMATRIX* getMatrix() = 0;
	~IMesh()
	{
		delete[] location;
	}
protected:
	virtual HRESULT Prepare() = 0;
	Graphics* G;
	Physx* PX;
	LPCWSTR location;
public:
	physx::PxActor* actor = NULL;
	UINT stride = 0, offset = 0;
	physx::PxVec3 Scale;
	CComPtr<ID3D11Buffer> VertexBuffer;
	CComPtr<ID3D11Buffer> IndexBuffer;
};

