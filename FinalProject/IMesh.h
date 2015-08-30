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
	DirectX::XMMATRIX* getMatrix()
	{
		if (actor->isRigidActor())
		{
			physx::PxRigidActor* rigid = (physx::PxRigidActor*)actor;
			physx::PxMat44 m = physx::PxMat44(rigid->getGlobalPose());
			DirectX::XMMATRIX* dm = &DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m));
			return dm;
		}
		return NULL;
	}
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
};

