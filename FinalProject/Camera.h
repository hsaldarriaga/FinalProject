#pragma once

#include <DirectXMath.h>
#include "Physx.h"
class Camera
{
	friend class Ctrl360;
public:
	Camera(Physx* physx);
	void SetCamera(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 Up);
	void SetParameters(float FovY, float AspectRatio, float NearZ, float FarZ);
	void setView();
	void SetProjection();
	void Update();
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
	/*
	X [0..1], Y[0..1]
	*/
	void MoveDirection(float dirX, float dirY, float magnitude);
	void MovePosition(float dirX, float dirY, float magnitude);
	float Fovy, AspectRatio, NearZ, FarZ;
	float HAngle, VAngle, radio;
	Physx* PX;
public:
	physx::PxCapsuleController* controller = NULL;
};

