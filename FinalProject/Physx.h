#pragma once
#include <PxPhysicsAPI.h>
#if WIN32 //Librerías para la arquitectura de 32 bits
#if _DEBUG
#pragma comment(lib, "PhysX3DEBUG_x86.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PhysX3CharacterKinematicDEBUG_x86.lib")
#pragma comment(lib, "PhysX3CookingDEBUG_x86.lib")
#pragma comment(lib, "PhysXProfileSDKDEBUG.lib")
#pragma comment(lib, "PxTaskDEBUG.lib")

#else
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysX3CharacterKinematic_x86.lib")
#pragma comment(lib, "PhysX3Cooking_x86.lib")
#pragma comment(lib, "PhysXProfileSDK.lib")
#pragma comment(lib, "PxTask.lib")
#endif
#else //Librerías para la arquitectura de 64 bits
#if _DEBUG
#pragma comment(lib, "PhysX3DEBUG_x64.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PhysX3CharacterKinematicDEBUG_x64.lib")
#pragma comment(lib, "PhysX3CookingDEBUG_x64.lib")
#pragma comment(lib, "PhysXProfileSDKDEBUG.lib")
#pragma comment(lib, "PxTaskDEBUG.lib")

#else
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysX3CharacterKinematic_x64.lib")
#pragma comment(lib, "PhysX3Cooking_x64.lib")
#pragma comment(lib, "PhysXProfileSDK.lib")
#pragma comment(lib, "PxTask.lib")
#endif
#endif


class Physx
{
public:
	Physx();
	bool Init(void**);
	~Physx();

private:
	//Crea los objetos escenciales para que funcione la sdk de PhysX
	bool CreateFundamental(void**);
	//Crea la escena que simula el mundo físico
	bool CreateScene();

	physx::PxFoundation* pFoundation;
	physx::PxProfileZoneManager* pProfileZoneManager;
	physx::PxCudaContextManager* pCudaContextManager;
	bool recordMemoryAllocations;
public:
	physx::PxPhysics* pPhysics;
	physx::PxCooking* pCooking;
	physx::PxScene* pPxScene;
	physx::PxControllerManager* controllerManager;
};

