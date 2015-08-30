#pragma once
#include <PxPhysicsAPI.h>

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

class Physx
{
public:
	Physx();
	bool Init(void**);
	~Physx();

private:

	bool CreateFundamental(void**);
	bool CreateScene();

	physx::PxFoundation* pFoundation = NULL;
	physx::PxProfileZoneManager* pProfileZoneManager = NULL;
	physx::PxCudaContextManager* pCudaContextManager = NULL;
	bool recordMemoryAllocations;
public:
	physx::PxPhysics* pPhysics = NULL;
	physx::PxCooking* pCooking = NULL;
	physx::PxScene* pPxScene = NULL;
	physx::PxControllerManager* controllerManager = NULL;
};

