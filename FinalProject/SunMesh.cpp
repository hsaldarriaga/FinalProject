#include "SunMesh.h"

using namespace::DirectX;

SunMesh::SunMesh(Graphics* g, Physx* px, LPCWSTR location) : IMesh(g, px, location)
{
	stride = sizeof(XMFLOAT3A);
}

bool SunMesh::Initialize(FbxNode* node)
{
	FbxNode* lChild = node->GetChild(0);
	FbxMesh * lMesh = lChild->GetMesh();
	if (lMesh)
	{
		FbxVector4* PVertices;
		int VertexCount = lMesh->GetPolygonCount();
		PVertices = lMesh->GetControlPoints();
		int index = 0;
		for (int i = 0; i < VertexCount; i++)
		{
			int face_index0 = lMesh->GetPolygonVertex(i, 0);
			int face_index1 = lMesh->GetPolygonVertex(i, 1);
			int face_index2 = lMesh->GetPolygonVertex(i, 2);
			FbxVector4 v = PVertices[face_index0];
			pIndexforD3D.push_back(index);
			pDataforD3D.push_back(XMLoadFloat3A(&XMFLOAT3A((float)v.mData[0], (float)v.mData[1], (float)v.mData[2]))); index++;
			v = PVertices[face_index1];
			pIndexforD3D.push_back(index);
			pDataforD3D.push_back(XMLoadFloat3A(&XMFLOAT3A((float)v.mData[0], (float)v.mData[1], (float)v.mData[2]))); index++;
			v = PVertices[face_index2];
			pIndexforD3D.push_back(index);
			pDataforD3D.push_back(XMLoadFloat3A(&XMFLOAT3A((float)v.mData[0], (float)v.mData[1], (float)v.mData[2]))); index++;
		}
		FbxAMatrix l = lChild->EvaluateLocalTransform();
		World = FbxToPx(l);
		FbxSurfacePhong* lMaterial = (FbxSurfacePhong*)lChild->GetMaterial(0);
		FbxDouble3 vec = lMaterial->Diffuse.Get();
		Kd = DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(vec.mData[0]), static_cast<FLOAT>(vec.mData[1]), static_cast<FLOAT>(vec.mData[2]), 1.0f));
		if (FAILED(Prepare()))
			return false;
		return true;
	}
	return false;
}

int SunMesh::getIndexesCount()
{
	return pIndexforD3D.size();
}

DirectX::XMMATRIX* SunMesh::getMatrix()
{
	return &DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&World));
}
void SunMesh::SetBuffers()
{
	ID3D11DeviceContext* d = G->pDevContext;
	D3D11_MAPPED_SUBRESOURCE sub;
	d->Map(cbSun_Params, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
	SUN_PARAMS* data = reinterpret_cast<SUN_PARAMS*>(sub.pData);
	data->Kd = Kd;
	d->Unmap(cbSun_Params, 0);
	d->PSSetConstantBuffers(0, 1, &cbSun_Params.p);
}
void SunMesh::SetParams(DirectX::XMFLOAT3* position)
{
	World.setPosition(physx::PxVec3(position->x, position->y, position->z));
}

HRESULT SunMesh::Prepare()
{
	D3D11_BUFFER_DESC ds;
	ds.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	ds.ByteWidth = pDataforD3D.size()*sizeof(XMFLOAT3A);
	ds.CPUAccessFlags = 0;
	ds.MiscFlags = 0;
	ds.StructureByteStride = sizeof(XMFLOAT3A);
	ds.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA dt;
	dt.pSysMem = pDataforD3D.data();
	dt.SysMemPitch = 0;
	dt.SysMemSlicePitch = 0;

	HRESULT hr = G->pDevice->CreateBuffer(&ds, &dt, &VertexBuffer);
	if (FAILED(hr)){ return hr; }

	ds.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ds.ByteWidth = pIndexforD3D.size()*sizeof(UINT);
	ds.StructureByteStride = sizeof(UINT);

	dt.pSysMem = pIndexforD3D.data();

	hr = G->pDevice->CreateBuffer(&ds, &dt, &IndexBuffer);

	ID3D11Device* g = G->pDevice;
	ZeroMemory(&ds, sizeof(D3D11_BUFFER_DESC));
	ds.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ds.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ds.Usage = D3D11_USAGE_DYNAMIC;
	ds.ByteWidth = sizeof(SUN_PARAMS);
	ds.StructureByteStride = sizeof(SUN_PARAMS);
	hr = g->CreateBuffer(&ds, NULL, &cbSun_Params);
	return hr;
}

SunMesh::~SunMesh()
{
}
