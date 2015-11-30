#include "Physx.h"

using namespace::physx;

Physx::Physx()
{
#if _DEBUG
	recordMemoryAllocations = true;
#else
	bool recordMemoryAllocations = false;
#endif
	pFoundation = NULL;
	pProfileZoneManager = NULL;
	pCudaContextManager = NULL;
	pPhysics = NULL;
	pCooking = NULL;
	pPxScene = NULL;
	controllerManager = NULL;
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
	//Código para inicializar la SDK de PhysX
	static PxDefaultAllocator allocatorcallback;
	static PxDefaultErrorCallback errorcallback;
	pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocatorcallback, errorcallback);
	if (!pFoundation)
		return false;
	pProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(pFoundation);
	if (!pProfileZoneManager)
		return false;
	//pPhysics es una clase primordial de donde se crean varios componenten para simular la física
	pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pFoundation, PxTolerancesScale(), recordMemoryAllocations, pProfileZoneManager);
	if (!pPhysics)
		return false;
	if (!PxInitExtensions(*pPhysics))
		return false;
	PxCookingParams params = PxCookingParams(PxTolerancesScale());
	//params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
	//params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	//params.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;
	//pCooking permite la manipulación de vértices de los modelos 3D en el modelo físico presentado por PhysX
	pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pFoundation, params);
	if (!pCooking)
		return false;
	PxCudaContextManagerDesc desc;
	desc.appGUID = "someguid";
	desc.graphicsDevice = *device;
	desc.interopMode = PxCudaInteropMode::D3D11_INTEROP;
	//Cuda es la tecnología que hace que las operaciones matemáticas para modelar el mundo físico se hagan
	//dentro de la GPU
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
		//Parámetro que indica a PxScene que use los núcleos CUDA solo presenten en tarjetas de video NVIDIA para hacer cálculos matemáticos
		scenedesc.gpuDispatcher = pCudaContextManager->getGpuDispatcher();
	}
	//pPxScene representa un mundo físico donde los modelos 3D interactuan ejerciendo fuerzas
	pPxScene = pPhysics->createScene(scenedesc);
	if (!pPxScene)
		return false;
	//controllerManager permite crear el modelo físico de la cámara
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
