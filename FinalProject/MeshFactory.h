#pragma once

#include<Windows.h>
#include "IMesh.h"
#include "Physx.h"

class MeshFactory
{
public:
	MeshFactory(Physx* physx, Graphics* g);
	//A partir de una ruta a un archivo .fbx, obtiene el modelo 3D
	IMesh* Initialize(LPWSTR filename, LPCSTR type);
	~MeshFactory();

private:
	FbxManager* lSdkManager;
	Physx* Physxsdk;
	Graphics* G;
};

