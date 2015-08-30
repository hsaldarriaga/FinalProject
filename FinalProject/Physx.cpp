#include "Physx.h"

using namespace::physx;

Physx::Physx()
{
#if _DEBUG
	recordMemoryAllocations = true;
#else
	bool recordMemoryAllocations = false;
#endif
}

bool Physx::Init(void** device) 
{
	if (!CreateFundamental(device))
		return false;
	if (!CreateScene())
		return false;
	return true;
}


bool Physx::CreateFundamental(void** device)
{
	static PxDefaultAllocator allocatorcallback;
	static PxDefaultErrorCallback errorcallback;
	pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocatorcallback, errorcallback);
	if (!pFoundation)
		return false;
	pProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(pFoundation);
	if (!pProfileZoneManager)
		return false;
	pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pFoundation, PxTolerancesScale(), recordMemoryAllocations, pProfileZoneManager);
	if (!pPhysics)
		return false;
	if (!PxInitExtensions(*pPhysics))
		return false;
	PxCookingParams params = PxCookingParams(PxTolerancesScale());
	//params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	//params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	//params.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;

	pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pFoundation, params);
	if (!pCooking)
		return false;
	PxCudaContextManagerDesc desc;
	desc.appGUID = "someguid";
	desc.graphicsDevice = *device;
	desc.interopMode = PxCudaInteropMode::D3D11_INTEROP;
	
	pCudaContextManager = PxCreateCudaContextManager(*pFoundation, desc, pProfileZoneManager);
	return true;
}

bool Physx::CreateScene()
{
	PxSceneDesc scenedesc = PxSceneDesc(PxTolerancesScale());
	if (!scenedesc.cpuDispatcher)
	{
		PxCpuDispatcher* dispatcher = PxDefaultCpuDispatcherCreate(3);
		if (!dispatcher)
			return false;
		scenedesc.cpuDispatcher = dispatcher;
	}
	if (!scenedesc.filterShader)
		scenedesc.filterShader = PxDefaultSimulationFilterShader;
	if (!scenedesc.gpuDispatcher && pCudaContextManager)
	{ 
		scenedesc.gpuDispatcher = pCudaContextManager->getGpuDispatcher();
	}
	pPxScene = pPhysics->createScene(scenedesc);
	if (!pPxScene)
		return false;
	controllerManager = PxCreateControllerManager(*pPxScene);
	if (!controllerManager)
		return false;
	controllerManager->setOverlapRecoveryModule(true);
	return true;
}

Physx::~Physx()
{
	if (controllerManager)
		controllerManager->release();
	if (pPxScene)
		pPxScene->release();
	if (pCudaContextManager)
		pCudaContextManager->release();
	if (pCooking)
		pCooking->release();
	if (pPhysics)
		pPhysics->release();
	if (pProfileZoneManager)
		pProfileZoneManager->release();
	if (pFoundation)
		pFoundation->release();
}
