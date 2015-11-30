#include "TerrainMesh.h"
#include "ImageFactory.h"

TerrainMesh::TerrainMesh(Graphics* g, Physx* px, LPCWSTR location) : IMesh(g, px, location)
{
	stride = sizeof(DataLayout);
	World = DirectX::XMMatrixIdentity();
}
//1.00614929
//max height = 0.0956463963
//min height = -0.0702662021
bool TerrainMesh::Initialize(FbxNode* node)
{
	FbxNode* lChild = node->GetChild(0);
	//Se obtienen los vértices
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
			//Se obtiene 3 índices que representa 1 triángulo del modelo 3D.
			int face_index0 = lMesh->GetPolygonVertex(i, 0);
			int face_index1 = lMesh->GetPolygonVertex(i, 1);
			int face_index2 = lMesh->GetPolygonVertex(i, 2);
			FbxVector4 face_nor_0, face_nor_1, face_nor_2;
			//Se obtiene los vectores normales de cada vértice
			lMesh->GetPolygonVertexNormal(i, 0, face_nor_0);
			lMesh->GetPolygonVertexNormal(i, 1, face_nor_1);
			lMesh->GetPolygonVertexNormal(i, 2, face_nor_2);
			//Se obtiene las coordenadas UV de cada vértice
			int face_uv_0 = lMesh->GetTextureUVIndex(i, 0);
			int face_uv_1 = lMesh->GetTextureUVIndex(i, 1);
			int face_uv_2 = lMesh->GetTextureUVIndex(i, 2);
			FbxVector2 uv = uvVertices->GetAt(face_uv_0);
			FbxVector4 v = PVertices[face_index0];
			pIndexforD3D.push_back(index);
			//Se obtiene el primer vértice del triangulo y se almacena en pDataforD3D con la estructra DataLayout
			pDataforD3D.push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_0.mData[0], (float)face_nor_0.mData[1], (float)face_nor_0.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
			v = PVertices[face_index1];
			uv = uvVertices->GetAt(face_uv_1);
			pIndexforD3D.push_back(index);
			//Se obtiene el segundo vértice del triangulo y se almacena en pDataforD3D con la estructra DataLayout
			pDataforD3D.push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_1.mData[0], (float)face_nor_1.mData[1], (float)face_nor_1.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
			v = PVertices[face_index2];
			uv = uvVertices->GetAt(face_uv_2);
			pIndexforD3D.push_back(index);
			//Se obtiene el tercero vértice del triangulo y se almacena en pDataforD3D con la estructra DataLayout
			pDataforD3D.push_back(DataLayout((float)v.mData[0], (float)v.mData[1], (float)v.mData[2], (float)face_nor_2.mData[0], (float)face_nor_2.mData[1], (float)face_nor_2.mData[2], (float)uv.mData[0], (float)uv.mData[1])); index++;
		}
		FbxSurfacePhong* lMaterial = (FbxSurfacePhong*)lChild->GetMaterial(0);
		FbxProperty prop = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(0));
		ImageFactory* fac = new ImageFactory(G->pDevice);
		//Se obtiene la ruta relativa de la textura que se va usar para pintar el modelo.
		const char* filename = texture->GetRelativeFileName();
		size_t pathsize = (wcslen(location) + 1) * 2;
		char rute[MAX_PATH];
		wcstombs_s(NULL, rute, pathsize, location, _TRUNCATE);
		LPSTR LongRute = lstrcatA(rute, "\\");
		//Se obtiene la textura
		HRESULT hr = fac->getImageFromFileName(lstrcatA(LongRute, filename), &resource);
		if (FAILED(hr)) {
			return false; delete fac;
		}
		char filenameHeightmap[MAX_PATH];
		strcpy_s(filenameHeightmap, "height_map_");
		lstrcatA(filenameHeightmap, filename);
		wcstombs_s(NULL, rute, pathsize, location, _TRUNCATE);
		LongRute = lstrcatA(rute, "\\");
		//Se obtiene otra textura que corresponde al HeightMap del terreno, usado por la clase Ocean para ajustar la transparencia del agua de acuerdo a la altura del terreno.
		hr = fac->getImageFromFileName(lstrcatA(LongRute, filenameHeightmap), &HeightMap);
		delete fac;
		if (FAILED(hr))
			return false;
		FbxAMatrix l = lChild->EvaluateLocalTransform();
		//Como los actores que se manejan en PhysX no pueden ser escalados con una matriz de escalamiento, se extrae los valores de escala.
		Scale.x = (float)l.Get(0, 0); Scale.y = (float)l.Get(1, 1); Scale.z = (float)l.Get(2, 2);
		//PxTransform es la estructura que usa Physx para hacer las transformaciones al modelo
		physx::PxTransform t = physx::PxTransform(physx::PxIdentity);
		FbxQuaternion Q = l.GetQ();
		FbxVector4 T = l.GetT();
		t.q = physx::PxQuat((float)Q.GetAt(0), (float)Q.GetAt(1), (float)Q.GetAt(2), (float)Q.GetAt(3));
		t.p = physx::PxVec3((float)T.mData[0], (float)T.mData[1], (float)T.mData[2]);
		//Se crear un actor rigido y estático, esto debido al que terreno no se mueve y siempre permanece estático
		actor = PX->pPhysics->createRigidStatic(t);
		FbxDouble3 vec = lMaterial->Ambient.Get();
		//Se obtiene el color de ambiente
		Ka = DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(vec.mData[0]), static_cast<FLOAT>(vec.mData[1]), static_cast<FLOAT>(vec.mData[2]), 1.0f));
		vec = lMaterial->Diffuse.Get();
		//Se obtiene el color de difusión
		Kd = DirectX::XMLoadFloat4(new DirectX::XMFLOAT4(static_cast<FLOAT>(vec.mData[0]), static_cast<FLOAT>(vec.mData[1]), static_cast<FLOAT>(vec.mData[2]), 1.0f));
		if (!AddPhysics())
			return false;
		if (FAILED(Prepare()))
			return false;
		return true;
	}
	return false;
}

ID3D11ShaderResourceView* TerrainMesh::getHeightMap()
{
	return HeightMap.p;
}

bool TerrainMesh::AddPhysics()
{
	//Se agregan los vértices al actor que se define en el mundo físico de PhysX para que reaccione a las colisiones.
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
	//se hace el proceso de lectura de los vértices para representar el actor
	if (!PX->pCooking->cookTriangleMesh(desc, writeBuffer))
		return false;
	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	physx::PxTriangleMesh* trimesh = PX->pPhysics->createTriangleMesh(readBuffer);
	physx::PxMeshScale sc = physx::PxMeshScale();
	sc.scale.x = Scale.x; sc.scale.y = Scale.y; sc.scale.z = Scale.z;
	//Como no se puede escalar el actor con una matriz de transfromación, entonces se aplica la escala a la geometría
	physx::PxTriangleMeshGeometry geometry = physx::PxTriangleMeshGeometry(trimesh, sc);
	//Cada actor del mundo físico debe tener un material
	physx::PxMaterial* material = PX->pPhysics->createMaterial(0.1f, 0.5f, 1.0f);
	//Se asigna la figura geometrica compuesto por triángulos al actor que representa este modelo.
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
	//Se almacena los vértices dentro de la GPU
	HRESULT hr = G->pDevice->CreateBuffer(&ds, &dt, &VertexBuffer.p);
	if (FAILED(hr)){ return hr; }

	ds.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ds.ByteWidth = pIndexforD3D.size()*sizeof(UINT);
	ds.StructureByteStride = sizeof(UINT);

	dt.pSysMem = pIndexforD3D.data();
	//Se almacenan los índices que indica el orden en que se pintan los vértices, dentro de la GPU
	hr = G->pDevice->CreateBuffer(&ds, &dt, &IndexBuffer.p);
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
		if (DirectX::XMMatrixIsIdentity(World)) {
			//Se obtiene la Matriz de World de éste modelo a partir de su actor de física
			physx::PxRigidActor* rigid = (physx::PxRigidActor*)actor;
			physx::PxMat44 m = physx::PxMat44(rigid->getGlobalPose());
			//Como bien lo dijimos, el actor no incluye la matriz de Escala, por lo tanto se asigna manualmente.
			m.column0.x = Scale.x;
			m.column1.y = Scale.y;
			m.column2.z = Scale.z;
			DirectX::XMFLOAT4X4A xm;
			xm._11 = m.column0.x; xm._12 = m.column0.y; xm._13 = m.column0.z; xm._14 = m.column0.w;
			xm._21 = m.column1.x; xm._22 = m.column1.y; xm._23 = m.column1.z; xm._24 = m.column1.w;
			xm._31 = m.column2.x; xm._32 = m.column2.y; xm._33 = m.column2.z; xm._34 = m.column2.w;
			xm._41 = m.column3.x; xm._42 = m.column3.y; xm._43 = m.column3.z; xm._44 = m.column3.w;
			World = DirectX::XMLoadFloat4x4A(&xm);
		}
		return &World;
	}
	return NULL;
}
TerrainMesh::~TerrainMesh()
{
	if (actor)
		actor->release();
}
