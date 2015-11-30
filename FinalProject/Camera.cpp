#include "Camera.h"

using namespace::physx;
using namespace::DirectX;

Camera::Camera(Physx* px)
{
	this->PX = px;
	View = XMMatrixIdentity();
	Proj = XMMatrixIdentity();
	this->ActiveJump = false;
	this->ylastposition = 0;
	this->jumptime = 0;
	this->controller = NULL;
}

void Camera::setPhysx(XMFLOAT3 eye)
{
	/*
	Se crea el material que está compuesto la cámara, eso incluye la fricción estática,
	fricción dinámica y la fuerza de restitución.
	*/
	PxMaterial* material = PX->pPhysics->createMaterial(0.2f, 0.5f, 0.0f);
	//Esta clase define la cámara como una capsula con altura de 5 unidades, un radio de 0.5
	//una posición eye dada, y movimiento mínimo de 0.8.
	PxCapsuleControllerDesc desc;
	desc.height = 5;
	desc.material = material;
	desc.position = PxExtendedVec3(eye.x, eye.y, eye.z);
	desc.radius = 0.5;
	desc.stepOffset = 0.8f;
	//Se procede a crear la capsula
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
	//Se obtiene la dirección de la cámara restando Target - eye
	XMFLOAT3 sb= XMFLOAT3(Target.x - eye.x, Target.y - eye.y, Target.z - eye.z);
	//Se inicializan los valores para poder mover la dirección de la cámara
	//Se sigue el modelo de coordenadas ésfericas en donde se usan 3 parámetros
	//El radio, Un angulo para girar horizontalmente (HAngle) el rango es de 0 a 2PI
	//Y el ángulo para girar verticalmente (VAngle) el rango es de 0 a PI.
	radio = sqrtf(sb.x*sb.x + sb.y*sb.y + sb.z*sb.z);
	VAngle = acosf(sb.y / radio);
	HAngle = acosf(sb.x / (radio * sinf(VAngle)));
	//Se hace un ligero movimiento para evitar un primer movimiento irregular
	//la primera vez que mueva la dirección de la cámara
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
	//Se crea la View matrix
	View = XMMatrixLookAtLH(Eye, Target, Up);
}

void Camera::SetProjection()
{
	//Se crea la Pespective Projection Matrix
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
	//Se cambia la dirección de dirX y dirY para que concuerda con el movimiento del análogo
	//del control. La magnitud es disminuida a un 5% para que el movimiento de la cámara no
	//sea muy rápida.
	dirX *= -0.05f*magnitude;
	dirY *= -0.05f*magnitude;
	//Se cuida que el ángulo vértical no sobrepase sus límites
	if (VAngle + dirY > 0 && VAngle + dirY < DirectX::XM_PI)
	{
		VAngle += dirY;
	}
	//Se verifica también que el ángulo horizontal no sobrepase sus límites, y en caso de que
	//pase, se le asigna un ángulo equivalente al valor pasado.
	if (HAngle + dirX > 2 * DirectX::XM_PI)
		HAngle = 0;
	else if (HAngle + dirX < 0)
		HAngle = 2 * DirectX::XM_PI;
	else
		HAngle += dirX;
	//Se convierten las coordenadas esféricas a coordenadas cartesianas en 3 dimensiones
	xo = radio * sinf(VAngle)*cosf(HAngle);
	yo = radio * cosf(VAngle);
	zo = radio * sinf(VAngle) * sinf(HAngle);
	XMFLOAT3 se;
	//El contenido de Eye se pasa a la variable 'se'
	XMStoreFloat3(&se, Eye);
	//Luego la nueva posición de Targe va estar definida por la posición de Eye + (x0,y0,z0)
	Target = XMLoadFloat3(&XMFLOAT3(se.x + xo, se.y + yo, se.z + zo));
}
/*
	-X left
	+X right
	+Y up
	-Y down
*/
void Camera::MovePosition(float dirX, float dirY, float magnitude)
{
	//Sino no se encuentra activamente en un salto
	if (!ActiveJump)
	{
		XMVECTOR sb = Target - Eye;
		XMFLOAT3 val; XMStoreFloat3(&val, sb); val.y = 0;
		sb = XMLoadFloat3(&val);
		//Obtiene a dirección de la cámara normalizada.
		sb = XMVector3Normalize(sb);
		//Producto cartesiano de la dirección de la cámara con el vector Up (0,1,0)
		//para determinar cual es la izquierda y cual es la derecha actual de la cámara.
		XMVECTOR dir = XMVector3Cross(sb, Up);
		sb *= dirY*magnitude;//Movimiento hacíia adelante o atrás dependiendo del signo de
		//dirX
		dir *= -dirX*magnitude;//Movimiento hacia la izquiera o la dereche, dependiendo del
		//signo de dirX
		XMFLOAT3 resu;
		XMStoreFloat3(&resu, sb + dir); //se combinan ambos movimientos
		lastdx = resu.x; lastdz = resu.z;
		//lastdx almacena los movimientos izquierda y derecha
		//lastdy almacena los movimientos adelante y atrás
	}
	//Como la velocidad de frencuencia de la pantalla es de 60 fotogramas por segundo
	// el tiempo de movimiento en milisegundo es de 1 / 60 * 1000
	if (controller->move(getMovement(), 0.01f, 1.0f / 60.0f * 1000.0f, filters) == PxControllerCollisionFlag::eCOLLISION_DOWN) //Se realiza el movimiento y se verifica si la cámara está tocando el suelo.
	{
		//Si está tocando el suelo, el salto ha acabado.
		ActiveJump = false; jumptime = 0; ylastposition = 0;
	}
}

void Camera::Update()
{
	//Actualiza la posición de la cámara con la representación física de la capsula.
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
	//Si el control no ha activado ningun movimiento, puede que esté en pleno salto o simplemente esté actuando la gravedad.
	if (controller->move(getMovement(), 0.01f, 1.0f / 60.0f * 1000.0f, filters) == PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		ActiveJump = false; jumptime = 0; ylastposition = 0;
	}
	
}

PxVec3 Camera::getMovement()
{
	//Si actualmente está en el proceso de salto
	if (ActiveJump)
	{
		//El tiempo de salto aumenta 0.01666 segundos por la velocidad de 60 fotogramas por segundo.
		jumptime = jumptime + 1.0f / 60.0f;
		//ecuación de movimiento parábolico
		float Yactual = -0.5f*250.0f*(jumptime*jumptime) + 80.0f * jumptime;
		float result = Yactual - ylastposition;
		///ylastposition mantiene la última posición en la coordenada Y.
		ylastposition = Yactual;
		//result contiene la variación de la altura en el último instante de tiempo
		//lastdx y lastdz la dirección en la que se movía la cámara en el momento del salto.
		return PxVec3(lastdx, result, lastdz);
	}
	else 
	{
		//Si activamente no hay salto simplemente, ejecuta los movimientos hechos
		//por el análogo y siempre moviendose la cámara hacía abajo por acción 
		//de la gravedad.
		return PxVec3(lastdx, -0.5f, lastdz);
	}
}

Camera::~Camera()
{	
	//Elimina la capsula
	if (controller)
		controller->release();
}
