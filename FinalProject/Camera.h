#pragma once
#include <DirectXMath.h>
#include "Physx.h"
class Camera
{
public:
	Camera(Physx* physx);
	void SetCamera(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 Up);
	void SetParameters(float FovY, float AspectRatio, float NearZ, float FarZ);
	void setView();
	void SetProjection();
	DirectX::XMMATRIX GetViewProj();
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
	~Camera();

	DirectX::XMMATRIX View, Proj;
	DirectX::XMVECTOR Eye, Target, Up;
private:
	void setPhysx(DirectX::XMFLOAT3);
	float Fovy, AspectRatio, NearZ, FarZ;
	Physx* PX;
public:
	physx::PxController* controller = NULL;
};

