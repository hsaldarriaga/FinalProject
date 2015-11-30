#pragma once
#include "IMesh.h"
#include <atlbase.h>
#include <vector>
#include "Definitions.h"
//CompoundMesh es muy similar a TerrainMesh la diferencia es que se definen varios
//conjuntos de vértices en vez de uno sólo.
class CompoundMesh : public IMesh
{
public:
	CompoundMesh(Graphics* g, Physx* px, LPCWSTR location);
	bool Initialize(FbxNode*);
	void Render(ID3D11Buffer* cbMatrices, ID3D11SamplerState* SSTexture, ID3D11Buffer* cbLight, DirectX::XMMATRIX* View, DirectX::XMMATRIX* Proj);
	//Se obtiene el VertexBuffer dependiendo de cual conjunto de vértices se refiere
	ID3D11Buffer* getVertexBuffer(int);
	//Se obtiene el IndexBuffer dependiendo de cual conjunto de vértices se refiere
	ID3D11Buffer* getIndexBuffer(int);
	//El Stride es el mismo en este caso sin importar a cual conjunto de vértices se refiera.
	UINT getStride(int);
	UINT getOffset(int);
	//Cada conjunto de vértice tiene un número determinado de índices.
	UINT getIndexesCount(int);
	//Cada conjunto de vértice tiene definido su WorldMatrix
	DirectX::XMMATRIX* getMatrix(int);
#pragma region Allocator
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
#pragma endregion
	~CompoundMesh();
private:
	HRESULT Prepare();
	bool PrepareShaders();
	bool PrepareBuffers();
	bool AddPhysics();
	CComPtr<ID3D11Buffer>* VertexBuffers, *IndexBuffers;
	DirectX::XMMATRIX* Worlds;
	CComPtr<ID3D11ShaderResourceView>* Resources;
	std::vector<DataLayout>* pDataforD3Ds;
	std::vector<UINT>* pIndexforD3Ds;
	CComPtr<ID3D11VertexShader> VShader;
	CComPtr<ID3D11PixelShader> PShader;

	CComPtr<ID3D11InputLayout> InputLayout;
public:
	//Arreglo de actores
	physx::PxActor** actors;
};

