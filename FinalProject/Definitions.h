#pragma once

#include <DirectXMath.h>
#include "Physx.h"

__declspec(align(16)) struct DataLayout {
	DirectX::XMFLOAT3 Vertice;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 coordUV;
	DataLayout(float v1, float v2, float v3, float n1, float n2, float n3, float uv1, float uv2)
	{
		Vertice = DirectX::XMFLOAT3(v1, v2, v3);
		Normal = DirectX::XMFLOAT3(n1, n2, n3);
		coordUV = DirectX::XMFLOAT2(uv1, uv2);
	}
};

static DirectX::XMMATRIX ToXm(const FbxAMatrix& pSrc)
{
	return DirectX::XMMatrixSet(
		static_cast<FLOAT>(pSrc.Get(0, 0)), static_cast<FLOAT>(pSrc.Get(0, 1)), static_cast<FLOAT>(pSrc.Get(0, 2)), static_cast<FLOAT>(pSrc.Get(0, 3)),
		static_cast<FLOAT>(pSrc.Get(1, 0)), static_cast<FLOAT>(pSrc.Get(1, 1)), static_cast<FLOAT>(pSrc.Get(1, 2)), static_cast<FLOAT>(pSrc.Get(1, 3)),
		static_cast<FLOAT>(pSrc.Get(2, 0)), static_cast<FLOAT>(pSrc.Get(2, 1)), static_cast<FLOAT>(pSrc.Get(2, 2)), static_cast<FLOAT>(pSrc.Get(2, 3)),
		static_cast<FLOAT>(pSrc.Get(3, 0)), static_cast<FLOAT>(pSrc.Get(3, 1)), static_cast<FLOAT>(pSrc.Get(3, 2)), static_cast<FLOAT>(pSrc.Get(3, 3)));
}

static physx::PxMat44 FbxToPx(const FbxAMatrix& pSrc)
{
	physx::PxReal values[] = { 
		static_cast<FLOAT>(pSrc.Get(0, 0)), static_cast<FLOAT>(pSrc.Get(0, 1)), static_cast<FLOAT>(pSrc.Get(0, 2)), static_cast<FLOAT>(pSrc.Get(0, 3)),
		static_cast<FLOAT>(pSrc.Get(1, 0)), static_cast<FLOAT>(pSrc.Get(1, 1)), static_cast<FLOAT>(pSrc.Get(1, 2)), static_cast<FLOAT>(pSrc.Get(1, 3)),
		static_cast<FLOAT>(pSrc.Get(2, 0)), static_cast<FLOAT>(pSrc.Get(2, 1)), static_cast<FLOAT>(pSrc.Get(2, 2)), static_cast<FLOAT>(pSrc.Get(2, 3)),
		static_cast<FLOAT>(pSrc.Get(3, 0)), static_cast<FLOAT>(pSrc.Get(3, 1)), static_cast<FLOAT>(pSrc.Get(3, 2)), static_cast<FLOAT>(pSrc.Get(3, 3)),
	};
	return physx::PxMat44(values);
}

static DirectX::XMVECTOR ToXmV(FbxDouble3& pSrc)
{
	return DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(pSrc.mData[0]), static_cast<FLOAT>(pSrc.mData[1]), static_cast<FLOAT>(pSrc.mData[2]), 1.0f));
}

__declspec(align(16)) struct VS_CONSTANT
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX ViewProj;
};
__declspec(align(16)) struct MATERIAL_LIGHT
{
	DirectX::XMVECTOR Ka; //ambient
	DirectX::XMVECTOR Ks; //specular
	DirectX::XMVECTOR Kd; //diffuse
	float shininess; // how large specular lights are
};
__declspec(align(16)) struct LIGHT
{
	DirectX::XMVECTOR color;
	DirectX::XMVECTOR dir;
};
__declspec(align(16)) struct EYE
{
	DirectX::XMVECTOR eye;
};