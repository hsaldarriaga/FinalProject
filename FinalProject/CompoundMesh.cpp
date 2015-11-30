#include "CompoundMesh.h"
#include "FileLoader.h"
#include "ImageFactory.h"

CompoundMesh::CompoundMesh(Graphics* g, Physx* px, LPCWSTR location) : IMesh(g, px, location)
{
	stride = sizeof(DataLayout);
	offset = 0;
	Worlds = NULL;
	VertexBuffers = NULL; IndexBuffers = NULL;
	Resources = NULL;
	pDataforD3Ds = NULL;
	pIndexforD3Ds = NULL;
	actors = NULL;
}

bool CompoundMesh::Initialize(FbxNode* node)
{
	//Muy similar al proceso de inicialización de TerrainMesh, solo que
	//se manejan varios conjuntos de vértices y no uno sólo, por lo tanto
	//la mayoría de los elementos son arreglos
	FbxNode* cp = node;
	MeshCount = 0;
	MeshCount = node->GetChildCount();
	VertexBuffers = new CComPtr<ID3D11Buffer>[MeshCount];
	IndexBuffers = new CComPtr<ID3D11Buffer>[MeshCount];
	Resources = new CComPtr<ID3D11ShaderResourceView>[MeshCount];
	pDataforD3Ds = new std::vector<DataLayout>[MeshCount];
	pIndexforD3Ds = new std::vector<UINT>[MeshCount];
	actors = new physx::PxActor*[MeshCount];
	Worlds = (DirectX::XMMATRIX*)_mm_malloc(sizeof(DirectX::XMMATRIX)*MeshCount, 16);
	for (size_t m = 0; m < MeshCount; m++)
	{
		//Los conjuntos de vértices se obtienen según la posición m
		FbxNode* lChild = node->GetChild(m);
		FbxMesh* lMesh = lChild->GetMesh();
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
				pIndexforD3Ds[m].push_back(index);
				pDataforD3Ds[m].push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_0.mData[0], (float)face_nor_0.mData[1], (float)face_nor_0.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
				v = PVertices[face_index1];
				uv = uvVertices->GetAt(face_uv_1);
				pIndexforD3Ds[m].push_back(index);
				pDataforD3Ds[m].push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_1.mData[0], (float)face_nor_1.mData[1], (float)face_nor_1.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
				v = PVertices[face_index2];
				uv = uvVertices->GetAt(face_uv_2);
				pIndexforD3Ds[m].push_back(index);
				pDataforD3Ds[m].push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_2.mData[0], (float)face_nor_2.mData[1], (float)face_nor_2.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
			}
			FbxSurfacePhong* lMaterial = (FbxSurfacePhong*)lChild->GetMaterial(0);
			FbxProperty prop = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
			FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(0));
			ImageFactory* fac = new ImageFactory(G->pDevice);
			const char* filename = texture->GetRelativeFileName();
			size_t pathsize = (wcslen(location) + 1) * 2;
			char rute[MAX_PATH];
			wcstombs_s(NULL, rute, pathsize, location, _TRUNCATE);
			LPSTR LongRute = lstrcatA(rute, "\\");
			HRESULT hr = fac->getImageFromFileName(lstrcatA(LongRute, filename), &Resources[m].p);
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
			actors[m] = PX->pPhysics->createRigidStatic(t);
			physx::PxRigidActor* rigid = (physx::PxRigidActor*)actors[m];
			physx::PxMat44 fm = physx::PxMat44(rigid->getGlobalPose());
			fm.column0.x = Scale.x;
			fm.column1.y = Scale.y;
			fm.column2.z = Scale.z;
			DirectX::XMFLOAT4X4A xm;
			xm._11 = fm.column0.x; xm._12 = fm.column0.y; xm._13 = fm.column0.z; xm._14 = fm.column0.w;
			xm._21 = fm.column1.x; xm._22 = fm.column1.y; xm._23 = fm.column1.z; xm._24 = fm.column1.w;
			xm._31 = fm.column2.x; xm._32 = fm.column2.y; xm._33 = fm.column2.z; xm._34 = fm.column2.w;
			xm._41 = fm.column3.x; xm._42 = fm.column3.y; xm._43 = fm.column3.z; xm._44 = fm.column3.w;
			Worlds[m] = DirectX::XMLoadFloat4x4A(&xm);
		}
	}
	if (!AddPhysics())
		return false;
	if (FAILED(Prepare()))
		return false;
	return true;
}

HRESULT CompoundMesh::Prepare()
{
	if (!PrepareShaders())
		return E_FAIL;
	if (!PrepareBuffers())
		return E_FAIL;
	return ERROR_SUCCESS;
}
bool CompoundMesh::PrepareShaders()
{
	ID3D11Device* g = G->pDevice;
	BYTE* Data;
	LONG Size;
	//Se obtiene el sombreador NormalVertexShader y NormalPixelShader
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/NormalVertexShader.cso", &Data, &Size)))
		return false;
	//Se define la posición del vértice, su normal, y las coordenadas UV
	D3D11_INPUT_ELEMENT_DESC desc[3] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (FAILED(g->CreateInputLayout(desc, 3, Data, Size, &InputLayout.p))) {
		delete[] Data;
		return false;
	}
	if (FAILED(g->CreateVertexShader(Data, Size, NULL, &VShader.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	if (FAILED(FileLoader::getDataAndSize(L"/shaders/NormalPixelShader.cso", &Data, &Size)))
		return false;
	if (FAILED(g->CreatePixelShader(Data, Size, NULL, &PShader.p))) {
		delete[] Data;
		return false;
	}
	delete[] Data;
	return true;
}
bool CompoundMesh::PrepareBuffers()
{
	for (size_t k = 0; k < MeshCount; k++) 
	{
		D3D11_BUFFER_DESC ds;
		ds.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ds.ByteWidth = pDataforD3Ds[k].size()*sizeof(DataLayout);
		ds.CPUAccessFlags = 0;
		ds.MiscFlags = 0;
		ds.StructureByteStride = sizeof(DataLayout);
		ds.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA dt;
		dt.pSysMem = pDataforD3Ds[k].data();
		dt.SysMemPitch = 0;
		dt.SysMemSlicePitch = 0;

		HRESULT hr = G->pDevice->CreateBuffer(&ds, &dt, &VertexBuffers[k].p);
		if (FAILED(hr)) { 
			return false; 
		}

		ds.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ds.ByteWidth = pIndexforD3Ds[k].size()*sizeof(UINT);
		ds.StructureByteStride = sizeof(UINT);

		dt.pSysMem = pIndexforD3Ds[k].data();

		hr = G->pDevice->CreateBuffer(&ds, &dt, &IndexBuffers[k].p);
		if (FAILED(hr)) { 
			return false; 
		}
	}
	return true;
}
UINT CompoundMesh::getIndexesCount(int v)
{
	return pIndexforD3Ds[v].size();
}
ID3D11Buffer* CompoundMesh::getVertexBuffer(int v)
{
	return VertexBuffers[v].p;
}
ID3D11Buffer* CompoundMesh::getIndexBuffer(int v)
{
	return IndexBuffers[v].p;
}
UINT CompoundMesh::getStride(int v)
{
	return stride;
}
UINT CompoundMesh::getOffset(int v)
{
	return offset;
}
DirectX::XMMATRIX* CompoundMesh::getMatrix(int v)
{
	return &Worlds[v];
}

bool CompoundMesh::AddPhysics()
{
	//Similar a TerrainMesh, pero se crena diferentes actores por cada conjunto de vértices
	for (size_t k = 0; k < MeshCount; k++)
	{
		physx::PxTriangleMeshDesc desc;
		desc.points.count = pDataforD3Ds[k].size();
		desc.points.data = pDataforD3Ds[k].data();
		desc.points.stride = sizeof(DataLayout);
		desc.triangles.count = pIndexforD3Ds[k].size() / 3;
		desc.triangles.data = pIndexforD3Ds[k].data();
		desc.triangles.stride = sizeof(UINT) * 3;
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
		physx::PxMaterial* material = PX->pPhysics->createMaterial(0.1f, 0.5f, 1.0f);
		((physx::PxRigidStatic*)actors[k])->createShape(geometry, *material);
	}
	return true;
}
void CompoundMesh::Render(ID3D11Buffer* cbMatrices, ID3D11SamplerState* SSTexture, ID3D11Buffer* cbLight, DirectX::XMMATRIX* View, DirectX::XMMATRIX* Proj)
{
	ID3D11DeviceContext* d = G->pDevContext;
	d->IASetInputLayout(InputLayout); 
	d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (size_t i = 0; i < MeshCount; i++)
	{
		D3D11_MAPPED_SUBRESOURCE sub;
		d->Map(cbMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		WORLD_VIEW_PROJ_BUFFER* data = reinterpret_cast<WORLD_VIEW_PROJ_BUFFER*>(sub.pData);
		data->World = DirectX::XMMatrixTranspose(this->Worlds[i]);// Se asigna la World Matrix de cada conjunto de vértices
		data->View = DirectX::XMMatrixTranspose(*View);
		data->Proj = DirectX::XMMatrixTranspose(*Proj);
		d->Unmap(cbMatrices, 0);

		d->IASetVertexBuffers(0, 1, &VertexBuffers[i].p, &stride, &offset);
		d->IASetIndexBuffer(IndexBuffers[i].p, DXGI_FORMAT_R32_UINT, 0);

		d->VSSetShader(VShader, NULL, 0);
		d->VSSetConstantBuffers(0, 1, &cbMatrices);
		d->PSSetShader(PShader, NULL, 0);
		d->PSSetShaderResources(0, 1, &Resources[i].p);
		d->PSSetSamplers(0, 1, &SSTexture);
		d->PSSetConstantBuffers(1, 1, &cbLight);

		d->DrawIndexed(pIndexforD3Ds[i].size(), 0, 0);
	}
}

CompoundMesh::~CompoundMesh()
{
	//Como algunos elementos son arreglos, hay que recorrerlos y eliminarlos uno por uno.
	for (size_t i = 0; i < MeshCount; i++) {
		if (actors) {
			actors[i]->release();
		}
		if (VertexBuffers) {
			VertexBuffers[i].Release();
		}
		if (IndexBuffers) {
			IndexBuffers[i].Release();
		}
		if (Resources) {
			Resources[i].Release();
		}
	}
	if (actors)
		delete[] actors;
	if (VertexBuffers)
		delete[] VertexBuffers;
	if (IndexBuffers)
		delete[] IndexBuffers;
	if (Resources)
		delete[] Resources;
	if (pDataforD3Ds)
		delete[] pDataforD3Ds;
	if (pIndexforD3Ds)
		delete[] pIndexforD3Ds;
	if (Worlds)
		_mm_free(Worlds);
}
