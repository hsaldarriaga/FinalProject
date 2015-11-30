#include "MeshFactory.h"
#include "TerrainMesh.h"
#include "CompoundMesh.h"
#include "SunMesh.h"
#include "SimpleMesh.h"
#include <Shlwapi.h>
//Librería necesaria para poder remover un archivo de una ruta en el disco
#pragma comment(lib, "Shlwapi.lib")

MeshFactory::MeshFactory(Physx* physxmanager, Graphics* g)
{
	this->G = g;
	this->Physxsdk = physxmanager;
	//Inicialización de la fbx SDK
	lSdkManager = FbxManager::Create();
	FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);
}

IMesh* MeshFactory::Initialize(LPWSTR filename, LPCSTR type)
{
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "importer");
	TCHAR* path = new TCHAR[MAX_PATH];
	DWORD Sizepath = MAX_PATH;
	DWORD val = GetModuleFileName(NULL, path, Sizepath);
	PathRemoveFileSpec(path);
	TCHAR* join = lstrcat(path, filename);
	const size_t length = (wcslen(join) + 1) * 2;
	char* resu = new char[(wcslen(join) + 1)*2];
	size_t converted;
	wcstombs_s(&converted, resu, length, join, _TRUNCATE);
	//Se lee el archivo .fbx
	if (!lImporter->Initialize(resu, -1, lSdkManager->GetIOSettings())) { //Si falla destruye la escena.
		lImporter->Destroy();
		delete[] join;
		delete[] resu;
		return NULL;
	}
	delete[] resu;
	PathRemoveFileSpec(join);
	FbxScene* pScene = FbxScene::Create(lSdkManager, "mainscene");
	lImporter->Import(pScene);
	lImporter->Destroy();
	FbxNode* lRootNode = pScene->GetRootNode(); //Se obtiene el nodo que contiene información de los vértices y las texturas
	if (lRootNode)
	{
		if (strcmp(type, "Terrain") == 0) { // Inicializa un modelo de tipo Terreno
			TerrainMesh* tm = new TerrainMesh(G, Physxsdk, join);
			if (!tm->Initialize(lRootNode))
			{
				delete tm;
				pScene->Destroy();
				return NULL;
			}
			pScene->Destroy();
			return tm;
		}
		else if (strcmp(type, "Sun") == 0) { //Inicializa el modelo 3D que correponde al sol
			SunMesh* sn = new SunMesh(G, Physxsdk, join);
			if (!sn->Initialize(lRootNode))
			{
				delete sn;
				pScene->Destroy();
				return NULL;
			}
			pScene->Destroy();
			return sn;
		}
		else if (strcmp(type, "Compound") == 0) //Inicializa un modelo 3D dividido en varios modelos 3D
		{
			CompoundMesh* cm = new CompoundMesh(G, Physxsdk, join);
			if (!cm->Initialize(lRootNode))
			{
				delete cm;
				pScene->Destroy();
				return NULL;
			}
			pScene->Destroy();
			return cm;
		}
		else if (strcmp(type, "Simple") == 0)//Un modelo 3D que tiene información del vértice, normal y coordenadas UV
		{
			SimpleMesh* sm = new SimpleMesh(G, Physxsdk, join);
			if (!sm->Initialize(lRootNode))
			{
				delete sm;
				pScene->Destroy();
				return NULL;
			}
			pScene->Destroy();
			return sm;
		}
		else
		{
			return NULL;
		}
	}
	pScene->Destroy();
	return NULL;
}

MeshFactory::~MeshFactory()
{
	if (lSdkManager)
		lSdkManager->Destroy();
}
