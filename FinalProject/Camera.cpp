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
	desc.height = 5;
	desc.material = material;
	desc.position = PxExtendedVec3(eye.x, eye.y, eye.z);
	desc.radius = 0.5;
	desc.stepOffset = 0.8f;
	controller = static_cast<PxCapsuleController*>(PX->controllerManager->createController(desc));
}

void Camera::SizeChange(float AspectRatio)
{
	this->AspectRatio = AspectRatio;
	SetProjection();
}

void Camera::SetCamera(XMFLOAT3 eye, XMFLOAT3 Target, XMFLOAT3 Up)
{
	setPhysx(eye);
	XMFLOAT3A data; data.x = eye.x; data.y = eye.y; data.z = eye.z;
	this->Eye = XMLoadFloat3A(&data);
	this->Target = XMLoadFloat3(&Target);
	this->Up = XMLoadFloat3(&Up);
	XMFLOAT3 sb= XMFLOAT3(Target.x - eye.x, Target.y - eye.y, Target.z - eye.z);
	radio = sqrtf(sb.x*sb.x + sb.y*sb.y + sb.z*sb.z);
	VAngle = acosf(sb.y / radio); // Y and Z exchange
	HAngle = acosf(sb.x / (radio * sinf(VAngle)));
	MoveDirection(0.0f, 1.0f, 0.0001f);
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

XMMATRIX Camera::GetView()
{
	return View;
}
XMMATRIX Camera::GetProj()
{
	return Proj;
}
XMVECTOR Camera::GetDir()
{
	return Target - Eye;
}
/*
	-X left
	+X right
	+Y up
	-Y down
*/
void Camera::MoveDirection(float dirX, float dirY, float magnitude)
{
	float xo, yo, zo;
	dirX *= -0.05f*magnitude;
	dirY *= -0.05f*magnitude;
	if (VAngle + dirY > 0 && VAngle + dirY < DirectX::XM_PI)
	{
		VAngle += dirY;
	}
	if (HAngle + dirX > 2 * DirectX::XM_PI)
		HAngle = 0;
	else if (HAngle + dirX < 0)
		HAngle = 2 * DirectX::XM_PI;
	else
		HAngle += dirX;
	xo = radio * sinf(VAngle)*cosf(HAngle);
	yo = radio * cosf(VAngle);
	zo = radio * sinf(VAngle) * sinf(HAngle);
	XMFLOAT3 se;
	XMStoreFloat3(&se, Eye);
	Target = XMLoadFloat3(&XMFLOAT3(se.x + xo, se.y + yo, se.z + zo));
}
void Camera::MovePosition(float dirX, float dirY, float magnitude)
{
	if (!ActiveJump)
	{
		XMVECTOR sb = Target - Eye;
		XMFLOAT3 val; XMStoreFloat3(&val, sb); val.y = 0;
		sb = XMLoadFloat3(&val);
		sb = XMVector3Normalize(sb);
		XMVECTOR dir = XMVector3Cross(sb, Up);
		sb *= dirY*magnitude*0.3f;
		dir *= -dirX*magnitude*0.3f;
		XMFLOAT3 resu;
		XMStoreFloat3(&resu, sb + dir);
		lastdx = resu.x; lastdz = resu.z;
	}
	if (controller->move(getMovement(), 0.01f, 1.0f / 60.0f * 1000.0f, filters) == PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		ActiveJump = false; jumptime = 0; ylastposition = 0;
	}
}

void Camera::Update()
{
	PxExtendedVec3 vec = controller->getPosition();
	XMFLOAT3 ceye, ctarget;
	XMStoreFloat3(&ceye, Eye);
	XMStoreFloat3(&ctarget, Target);
	XMFLOAT3 sb = XMFLOAT3(ctarget.x - ceye.x, ctarget.y - ceye.y, ctarget.z - ceye.z);
	Eye = XMLoadFloat3(&XMFLOAT3((float)vec.x, (float)vec.y, (float)vec.z));
	Target = XMLoadFloat3(&XMFLOAT3((float)(vec.x + sb.x), (float)(vec.y + sb.y), (float)(vec.z + sb.z)));
}

void Camera::Jump()
{
	ActiveJump = true;
}

void Camera::MoveIdle()
{
	if (!ActiveJump)
		lastdx = 0; lastdz = 0;
	if (controller->move(getMovement(), 0.01f, 1.0f / 60.0f * 1000.0f, filters) == PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		ActiveJump = false; jumptime = 0; ylastposition = 0;
	}
	
}

PxVec3 Camera::getMovement()
{
	if (ActiveJump)
	{
		jumptime = jumptime + 1.0f / 60.0f;
		float Yactual = -0.5f*150.0f*(jumptime*jumptime) + 80.0f * jumptime;
		float result = Yactual - ylastposition;
		ylastposition = Yactual;
		return PxVec3(lastdx, result, lastdz);
	}
	else 
	{
		return PxVec3(lastdx, -0.5f, lastdz);
	}
}

Camera::~Camera()
{
	if (controller)
		controller->release();
}
