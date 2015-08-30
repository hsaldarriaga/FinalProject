#include "Camera.h"

using namespace::physx;
using namespace::DirectX;

Camera::Camera(Physx* px)
{
	this->PX = px;
	View = XMMatrixIdentity();
	Proj = XMMatrixIdentity();
}

void Camera::setPhysx(XMFLOAT3 eye)
{
	PxMaterial* material = PX->pPhysics->createMaterial(0.2f, 0.5f, 0.0f);
	PxCapsuleControllerDesc desc;
	desc.height = 2;
	desc.material = material;
	desc.position = PxExtendedVec3(eye.x, eye.y, eye.z);
	desc.radius = 0.5;
	controller = PX->controllerManager->createController(desc);
}

void Camera::SetCamera(XMFLOAT3 eye, XMFLOAT3 Target, XMFLOAT3 Up)
{
	setPhysx(eye);
	XMFLOAT3A data; data.x = eye.x; data.y = eye.y; data.z = eye.z;
	this->Eye = XMLoadFloat3A(&data);
	this->Target = XMLoadFloat3(&Target);
	this->Up = XMLoadFloat3(&Up);
}

void Camera::SetParameters(float FovY, float AspectRatio, float NearZ, float FarZ)
{
	this->Fovy = FovY;
	this->AspectRatio = AspectRatio;
	this->NearZ = NearZ;
	this->FarZ = FarZ;
}

void Camera::setView()
{
	View = XMMatrixLookAtLH(Eye, Target, Up);
}

void Camera::SetProjection()
{
	Proj = XMMatrixPerspectiveFovLH(Fovy, AspectRatio, NearZ, FarZ);
}

XMMATRIX Camera::GetViewProj()
{
	return XMMatrixTranspose(XMMatrixMultiply(View, Proj));
}

Camera::~Camera()
{
	if (controller)
		controller->release();
}
