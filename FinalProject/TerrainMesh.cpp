#include "TerrainMesh.h"
#include "ImageFactory.h"

TerrainMesh::TerrainMesh(Graphics* g, Physx* px, LPCWSTR location) : IMesh(g, px, location)
{
	stride = sizeof(DataLayout);
}

bool TerrainMesh::Initialize(FbxNode* node)
{
	FbxNode* lChild = node->GetChild(0);
	FbxMesh * lMesh = lChild->GetMesh();
	if (lMesh)
	{
		FbxVector4* PVertices;
		int VertexCount = lMesh->GetPolygonCount();
		PVertices = lMesh->GetControlPoints();
		FbxLayerElementArrayTemplate<FbxVector2>* uvVertices = 0;
		lMesh->GetTextureUV(&uvVertices, FbxLayerElement::eTextureDiffuse);
		int index = 0;
		for (int i = 0; i < VertexCount; i++)
		{
			int face_index0 = lMesh->GetPolygonVertex(i, 0);
			int face_index1 = lMesh->GetPolygonVertex(i, 1);
			int face_index2 = lMesh->GetPolygonVertex(i, 2);
			FbxVector4 face_nor_0, face_nor_1, face_nor_2;
			lMesh->GetPolygonVertexNormal(i, 0, face_nor_0);
			lMesh->GetPolygonVertexNormal(i, 1, face_nor_1);
			lMesh->GetPolygonVertexNormal(i, 2, face_nor_2);
			int face_uv_0 = lMesh->GetTextureUVIndex(i, 0);
			int face_uv_1 = lMesh->GetTextureUVIndex(i, 1);
			int face_uv_2 = lMesh->GetTextureUVIndex(i, 2);
			FbxVector2 uv = uvVertices->GetAt(face_uv_0);
			FbxVector4 v = PVertices[face_index0];
			pIndexforD3D.push_back(index);
			pDataforD3D.push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_0.mData[0], (float)face_nor_0.mData[1], (float)face_nor_0.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
			v = PVertices[face_index1];
			uv = uvVertices->GetAt(face_uv_1);
			pIndexforD3D.push_back(index);
			pDataforD3D.push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_1.mData[0], (float)face_nor_1.mData[1], (float)face_nor_1.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
			v = PVertices[face_index2];
			uv = uvVertices->GetAt(face_uv_2);
			pIndexforD3D.push_back(index);
			pDataforD3D.push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_2.mData[0], (float)face_nor_2.mData[1], (float)face_nor_2.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
		}
		FbxSurfacePhong* lMaterial = (FbxSurfacePhong*)lChild->GetMaterial(0);
		FbxProperty prop = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(0));
		ImageFactory* fac = new ImageFactory(G->pDevice);
		const char* filename = texture->GetRelativeFileName();
		size_t pathsize = (wcslen(location) + 1) * 2;
		char* rute = new char[pathsize];
		wcstombs_s(NULL, rute, pathsize, location, _TRUNCATE);
		HRESULT hr = fac->getImageFromFileName(lstrcatA(lstrcatA(rute, "\\"), filename), &resource);
		delete[] rute;
		delete fac;
		if (FAILED(hr))
			return false;
		FbxAMatrix l = lChild->EvaluateLocalTransform();
		Scale.x = (float)l.Get(0, 0); Scale.y = (float)l.Get(1, 1); Scale.z = (float)l.Get(2, 2);
		physx::PxTransform t = physx::PxTransform(physx::PxIdentity);
		FbxQuaternion Q = l.GetQ();
		FbxVector4 T = l.GetT();
		t.q = physx::PxQuat((float)Q.GetAt(0), (float)Q.GetAt(1), (float)Q.GetAt(2), (float)Q.GetAt(3));
		t.p = physx::PxVec3((float)T.mData[0], (float)T.mData[1], (float)T.mData[2]);
		actor = PX->pPhysics->createRigidStatic(t);
		FbxDouble3 vec = lMaterial->Ambient.Get();
		Ka = DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(vec.mData[0]), static_cast<FLOAT>(vec.mData[1]), static_cast<FLOAT>(vec.mData[2]), 1.0f));
		//vec = lMaterial->Specular.Get();
		//Ks = DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(vec.mData[0]), static_cast<FLOAT>(vec.mData[1]), static_cast<FLOAT>(vec.mData[2]), 1.0f));
		vec = lMaterial->Diffuse.Get();
		Kd = DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(vec.mData[0]), static_cast<FLOAT>(vec.mData[1]), static_cast<FLOAT>(vec.mData[2]), 1.0f));
		//shininess = (float)lMaterial->Shininess.Get();
		if (!AddPhysics())
			return false;
		if (FAILED(Prepare()))
			return false;
		return true;
	}
	return false;
}

bool TerrainMesh::AddPhysics()
{
	physx::PxTriangleMeshDesc desc;
	desc.points.count = pDataforD3D.size();
	desc.points.data = pDataforD3D.data();
	desc.points.stride = sizeof(DataLayout);
	desc.triangles.count = pIndexforD3D.size()/3;
	desc.triangles.data = pIndexforD3D.data();
	desc.triangles.stride = sizeof(UINT)*3;
	physx::PxDefaultMemoryOutputStream writeBuffer;
#ifdef _DEBUG
	//mesh should be validated before cooked without the mesh cleaning
	bool res = PX->pCooking->validateTriangleMesh(desc);
	PX_ASSERT(res);
#endif
	if (!PX->pCooking->cookTriangleMesh(desc, writeBuffer))
		return false;
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	physx::PxTriangleMesh* trimesh = PX->pPhysics->createTriangleMesh(readBuffer);
	physx::PxMeshScale sc = physx::PxMeshScale();
	sc.scale.x = Scale.x; sc.scale.y = Scale.y; sc.scale.z = Scale.z;
	physx::PxTriangleMeshGeometry geometry = physx::PxTriangleMeshGeometry(trimesh, sc);
	physx::PxMaterial* material = PX->pPhysics->createMaterial(0.f, 0.5f, 1.0f);
	((physx::PxRigidStatic*)actor)->createShape(geometry, *material);
	return true;
}

HRESULT TerrainMesh::Prepare()
{
	D3D11_BUFFER_DESC ds;
	ds.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	ds.ByteWidth = pDataforD3D.size()*sizeof(DataLayout);
	ds.CPUAccessFlags = 0;
	ds.MiscFlags = 0;
	ds.StructureByteStride = sizeof(DataLayout);
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
	return hr;
}

inline int TerrainMesh::getIndexesCount()
{
	return pIndexforD3D.size();
}
DirectX::XMMATRIX* TerrainMesh::getMatrix()
{
	if (actor->isRigidActor())
	{
		physx::PxRigidActor* rigid = (physx::PxRigidActor*)actor;
		physx::PxMat44 m = physx::PxMat44(rigid->getGlobalPose());
		m.column0.x = Scale.x;
		m.column1.y = Scale.y;
		m.column2.z = Scale.z;
		DirectX::XMMATRIX* dm = &DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&m));
		return dm;
	}
	return NULL;
}
TerrainMesh::~TerrainMesh()
{
	if (actor)
		actor->release();
}
