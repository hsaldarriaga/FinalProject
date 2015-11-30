#pragma once
#include <Windows.h>
#include "Graphics.h"
#include "Physx.h"
#include <fbxsdk.h>
#include <DirectXMath.h>
//Librería estática necesaria para que sirva la FBX SDK
#pragma comment(lib, "libfbxsdk-mt.lib")

class IMesh
{
public:
	IMesh(Graphics* g, Physx* px, LPCWSTR location) 
	{ 
		this->G = g; this->PX = px; this->location = location;  
		this->actor = NULL; this->stride = 0; this->offset = 0;
	}
	//Conjunto de funciones virtuales que implementan el tipo de modelo 3D a que referencia
	virtual bool Initialize(FbxNode*) = 0;
	virtual int getIndexesCount() { return NULL; };
	virtual DirectX::XMMATRIX* getMatrix() { return NULL; };
	virtual DirectX::XMMATRIX* getMatrix(int) { return NULL; };
	virtual ID3D11Buffer* getVertexBuffer(int) { return NULL; };
	virtual ID3D11Buffer* getIndexBuffer(int) { return NULL; };
	virtual UINT getStride(int) { return NULL; };
	virtual UINT getOffset(int) { return NULL; };
	virtual UINT getIndexesCount(int) { return NULL; };
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
	physx::PxActor* actor;
	UINT stride, offset, MeshCount = 1;
	physx::PxVec3 Scale;
	CComPtr<ID3D11Buffer> VertexBuffer;
	CComPtr<ID3D11Buffer> IndexBuffer;
};

