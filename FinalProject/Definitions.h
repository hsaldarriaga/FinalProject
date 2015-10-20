#pragma once

#include <DirectXMath.h>
#include "Physx.h"
#include <fbxsdk.h>

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

__declspec(align(16)) struct WORLD_VIEW_PROJ_BUFFER
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
};

__declspec(align(16)) struct WORLD_VIEWxPROJ_BUFFER
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX ViewProj;
};
__declspec(align(16)) struct MATERIAL_LIGHT
{
	DirectX::XMVECTOR Ka; //ambient
	DirectX::XMVECTOR Kd; //diffuse
};
__declspec(align(16)) struct LIGHT_VIEW_PROJ_BUFFER
{
	DirectX::XMMATRIX LightView;
	DirectX::XMMATRIX LightProj;
	DirectX::XMVECTOR LightPos;
};
__declspec(align(16)) struct LIGHT
{
	DirectX::XMVECTOR color;
	DirectX::XMVECTOR dir;
};
__declspec(align(16)) struct LIGHTDIR_EYEDIR
{
	DirectX::XMVECTOR light_dir;
	DirectX::XMVECTOR eye_dir;
};
__declspec(align(16)) struct EYE
{
	DirectX::XMVECTOR eye;
};
__declspec(align(16)) struct VERTEXENTRY
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 textureuv;
	VERTEXENTRY(float posx, float posy, float posz, float texu, float texv)
	{
		position = DirectX::XMFLOAT3A(posx, posy, posz);
		textureuv = DirectX::XMFLOAT2A(texu, texv);
	}
};
__declspec(align(16)) struct CONSTANTFLOAT
{
	float value;
	float padd1 = 0;
	float padd2 = 0;
	float padd3 = 0;
	CONSTANTFLOAT(float value)
	{
		this->value = value;
	}
};
