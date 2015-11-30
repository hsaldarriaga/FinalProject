#pragma once
#include "Camera.h"
#include <Windows.h>
#include <Xinput.h>
//Dependiendo si se está compilando en un sistema operativo Windows 8 o Windows 7 la librería estática puede cambiar.
//#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	//#pragma comment(lib, "Xinput.lib")
//#else 
	#pragma comment(lib, "Xinput9_1_0.lib")
//#endif

#define LEFT_THUMB_DEADZONE  13000
#define RIGHT_THUMB_DEADZONE 13000
#define TRIGGER_THRESHOLD    30

class Ctrl360
{
public:
	Ctrl360(Camera*);
	void Init();
	//Monitorea si el usuario movió los análogos o presionó algún boton
	void CaptureUserInteractions();
	~Ctrl360();
private:
	bool Connect();
	//Administra el movimiento de los análogos
	void ManageThumbs(XINPUT_STATE* state);
	XINPUT_STATE oldstate;
	//Indica la numeración de cual control se está usando debido
	//a que se pueden usar hasta 4 controles al mismo tiempo.
	DWORD control_index;
	Camera* Cmr;
};

