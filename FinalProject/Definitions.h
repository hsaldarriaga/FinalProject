#pragma once

#include <DirectXMath.h>
#include "Physx.h"
#include <fbxsdk.h>

//__declspec se encarga de ajustar la estructura para que su tamaño sea múltiplo de 16 sin necesidad
//de agregar variables basura.
//DataLayout es la estructura que definen el modelo de los vértice de algún modelo 3D
__declspec(align(16)) struct DataLayout {
	DirectX::XMFLOAT3 Vertice; //X,Y,Z posición de los vértices que componen el modelo 3D
	DirectX::XMFLOAT3 Normal; // Normal del vértice 
	DirectX::XMFLOAT2 coordUV; //Coordenada UV del vértice
	DataLayout(float v1, float v2, float v3, float n1, float n2, float n3, float uv1, float uv2)
	{
		Vertice = DirectX::XMFLOAT3(v1, v2, v3);
		Normal = DirectX::XMFLOAT3(n1, n2, n3);
		coordUV = DirectX::XMFLOAT2(uv1, uv2);
	}
};
//Convierte una matriz definida en FBX SDK a una Matriz utilizable con la API de DirectX
static DirectX::XMMATRIX ToXm(const FbxAMatrix& pSrc)
{
	return DirectX::XMMatrixSet(
		static_cast<FLOAT>(pSrc.Get(0, 0)), static_cast<FLOAT>(pSrc.Get(0, 1)), static_cast<FLOAT>(pSrc.Get(0, 2)), static_cast<FLOAT>(pSrc.Get(0, 3)),
		static_cast<FLOAT>(pSrc.Get(1, 0)), static_cast<FLOAT>(pSrc.Get(1, 1)), static_cast<FLOAT>(pSrc.Get(1, 2)), static_cast<FLOAT>(pSrc.Get(1, 3)),
		static_cast<FLOAT>(pSrc.Get(2, 0)), static_cast<FLOAT>(pSrc.Get(2, 1)), static_cast<FLOAT>(pSrc.Get(2, 2)), static_cast<FLOAT>(pSrc.Get(2, 3)),
		static_cast<FLOAT>(pSrc.Get(3, 0)), static_cast<FLOAT>(pSrc.Get(3, 1)), static_cast<FLOAT>(pSrc.Get(3, 2)), static_cast<FLOAT>(pSrc.Get(3, 3)));
}
//Convierte una matriz definida en FBX SDK y la convierte a una matriz en PhysX
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
//Convierte un vector definido en FBX SDK a una estructura utilizable para DirectX
static DirectX::XMVECTOR ToXmV(FbxDouble3& pSrc)
{
	return DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(pSrc.mData[0]), static_cast<FLOAT>(pSrc.mData[1]), static_cast<FLOAT>(pSrc.mData[2]), 1.0f));
}

//Estructura que define las matrices de Transformación World, View, Proj
__declspec(align(16)) struct WORLD_VIEW_PROJ_BUFFER
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
};
//Estructura que define las matrices de Transformación World, View x Proj
__declspec(align(16)) struct WORLD_VIEWxPROJ_BUFFER
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX ViewProj;
};
//Estructura que define los colores utilizados para la iluminación
__declspec(align(16)) struct MATERIAL_LIGHT
{
	DirectX::XMVECTOR Ka; //ambient
	DirectX::XMVECTOR Kd; //diffuse
};
//Estructura que define las matrices de Transformación View, Proj cuando se hace desde la fuente de luz (sol)
__declspec(align(16)) struct LIGHT_VIEW_PROJ_BUFFER
{
	DirectX::XMMATRIX LightView;
	DirectX::XMMATRIX LightProj;
	DirectX::XMVECTOR LightPos;
};
//Estructura que guarda la dirección y el color de la luz del sol
__declspec(align(16)) struct LIGHT
{
	DirectX::XMVECTOR color;
	DirectX::XMVECTOR dir;
};
//Estructura que guarda la dirección de la luz y la dirección donde mira la cámara
__declspec(align(16)) struct LIGHTDIR_EYEDIR
{
	DirectX::XMVECTOR light_dir;
	DirectX::XMVECTOR eye_dir;
};
//Estructura que contiene la posición de la cámara
__declspec(align(16)) struct EYE
{
	DirectX::XMVECTOR eye;
};
//Estructura que define un conjunto de vértices que tiene la posición y la coordenada UV
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
//Estructura que define un simple valor de coma flotante
__declspec(align(16)) struct CONSTANTFLOAT
{
	float value;
	CONSTANTFLOAT(float value)
	{
		this->value = value;
	}
};
