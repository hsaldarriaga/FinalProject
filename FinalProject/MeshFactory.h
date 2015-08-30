#pragma once

#include<Windows.h>
#include "IMesh.h"
#include "Physx.h"

class MeshFactory
{
public:
	MeshFactory(Physx* physx, Graphics* g);
	IMesh* Initialize(LPWSTR filename, LPCSTR type);
	~MeshFactory();

private:
	FbxManager* lSdkManager = NULL;
	Physx* Physxsdk;
	Graphics* G;
};

