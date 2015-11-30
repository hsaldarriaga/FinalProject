#pragma once

#include <DirectXMath.h>
#include "Physx.h"
class Camera
{
	//Permite a las siguientes clases acceder a las funciones y variables privadas de Camera
	friend class Ctrl360;
	friend class ShadowMapping;
	friend class MotionBlurRender;
public:
	Camera(Physx* physx);
	//Obtiene los parámetros para crear la View Matrix
	void SetCamera(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 Up);
	//Obtiene los parámetros para construir la Perspective Projection Matrix
	void SetParameters(float FovY, float AspectRatio, float NearZ, float FarZ);
	//Construye la View Matrix
	void setView();
	//Construye la Pespective Projection Matrix
	void SetProjection();
	//Recontruye la Pesspective Projection Matrix cuando el AspectRatio cambia.
	void SizeChange(float AspectRatio);
	//Actualiza la posición de la cámara dependiendo del estado en que esté la cámara.
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
	//función que crea un modelo físico de la cámara para que reaccione a las colisiones.
	void setPhysx(DirectX::XMFLOAT3);
	/*
	Mueve la dirección de la cámara dependiendo de la dirección (dirX, dirY) y la velocidad
	a la que se produce el movimiento es determinado por magnitude.
	X [0..1], Y[0..1]
	*/
	void MoveDirection(float dirX, float dirY, float magnitude);
	//Mueve fisicamente la cámara en dirección (dirX, dirY) con una magnitud de magnitude
	void MovePosition(float dirX, float dirY, float magnitude);
	//Obtiene el vector que indica la dirección a la que actualmente se mueve la cámara
	physx::PxVec3 getMovement();
	/*
	Cuando no se ha ejercido ninguna acción sobre la cámara, la única fuerza que interviene es
	la gravedad, esta función matiene ejerciendo la fuerza de la gravedad cuando la cámara
	está en estado de reposo.
	*/
	void MoveIdle();
	//Hace la acción de salto a través de un movimiento parabólico.
	void Jump();
	bool ActiveJump;
	//Almacenan la variación de la posición que tuvo la cámara.
	float ylastposition, lastdx, lastdz;
	//Almacena el tiempo de salto
	float jumptime;
	//Variables que intervienen en el proceso de construcción de la View y Projection Matrix
	float Fovy, AspectRatio, NearZ, FarZ;
	float HAngle, VAngle, radio;
	Physx* PX;
	//Filtro vacío que se le asigna a las colisiones que tiene la cámara.
	physx::PxControllerFilters filters;
public:
	//Objeto que contiene las funciones del movimiento de la cámara.
	physx::PxCapsuleController* controller;
};

