#pragma once

#include <DirectXMath.h>
#include "Physx.h"
class Camera
{
	friend class Ctrl360;
	friend class ShadowMapping;
	friend class MotionBlurRender;
public:
	Camera(Physx* physx);
	void SetCamera(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 Up);
	void SetParameters(float FovY, float AspectRatio, float NearZ, float FarZ);
	void setView();
	void SetProjection();
	void SizeChange(float AspectRatio);
	void Update();
	DirectX::XMMATRIX GetView();
	DirectX::XMMATRIX GetProj();
	DirectX::XMVECTOR GetDir();
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
	physx::PxVec3 getMovement();
	void MoveIdle();
	//Jump
	void Jump();
	bool ActiveJump = false;
	float ylastposition = 0, lastdx, lastdz;
	float jumptime = 0;
	//Jump
	float Fovy, AspectRatio, NearZ, FarZ;
	float HAngle, VAngle, radio;
	Physx* PX;
	physx::PxControllerFilters filters;
public:
	physx::PxCapsuleController* controller = NULL;
};

