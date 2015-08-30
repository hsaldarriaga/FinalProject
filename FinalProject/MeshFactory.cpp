#include "MeshFactory.h"
#include "TerrainMesh.h"
#include <PathCch.h>

#pragma comment(lib, "Pathcch.lib")

MeshFactory::MeshFactory(Physx* physxmanager, Graphics* g)
{
	this->G = g;
	this->Physxsdk = physxmanager;
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
	PathCchRemoveFileSpec(path, Sizepath);
	TCHAR* join = lstrcat(path, filename);
	const size_t length = (wcslen(join) + 1) * 2;
	char* resu = new char[(wcslen(join) + 1)*2];
	size_t converted;
	wcstombs_s(&converted, resu, length, join, _TRUNCATE);
	if (!lImporter->Initialize(resu, -1, lSdkManager->GetIOSettings())) {
		lImporter->Destroy();
		delete[] join;
		delete[] resu;
		return NULL;
	}
	delete[] resu;
	PathCchRemoveFileSpec(join, Sizepath);
	FbxScene* pScene = FbxScene::Create(lSdkManager, "mainscene");
	lImporter->Import(pScene);
	lImporter->Destroy();
	FbxNode* lRootNode = pScene->GetRootNode();
	if (lRootNode)
	{
		if (type == "Terrain") {
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
