#include "Ctrl360.h"


Ctrl360::Ctrl360(Camera* cmr)
{
	this->Cmr = cmr;
	ZeroMemory(&oldstate, sizeof(XINPUT_STATE));
}


void Ctrl360::Init()
{
	Connect();
}

bool Ctrl360::Connect()
{
	DWORD dwResult;
	for (DWORD i = 0; i< XUSER_MAX_COUNT; i++)
	{
		ZeroMemory(&oldstate, sizeof(XINPUT_STATE));

		// Simplemente se obtienee el estado del control
		dwResult = XInputGetState(i, &oldstate);
		//Si se obtuvo el estado correctamente, el control está listo para usarse.
		if (dwResult == ERROR_SUCCESS)
		{
			XINPUT_CAPABILITIES caps;
			//Se verifica que el control que está conectado es un control de Xbox 360 y no tal vez un mando de guitarra.
			dwResult = XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &caps);
			if (dwResult == ERROR_SUCCESS){
				if (caps.SubType == XINPUT_DEVSUBTYPE_GAMEPAD){
					control_index = i;
					return true;
				}
			}
		}
	}
	return false;
}

void Ctrl360::CaptureUserInteractions()
{
	XINPUT_STATE state;
	DWORD dwResult = XInputGetState(control_index, &state);
	//Siempre se verifica que el control esté conectado
	if (dwResult == ERROR_SUCCESS)
	{
		//Si el estado del control ha cambiado(Algún botón fue presionado)
		if (state.dwPacketNumber != oldstate.dwPacketNumber && state.Gamepad.wButtons != 0)
		{
			//Si se presionó el botón A
			if (state.Gamepad.wButtons == XINPUT_GAMEPAD_A)
			{
				//Activar el salto de la cámara
				Cmr->Jump();
			}
		}
		//Se administran las acciones de los análogos
		ManageThumbs(&state);
		oldstate = state;
	}
	else {
		Connect();
	}
}

void Ctrl360::ManageThumbs(XINPUT_STATE* state)
{
	//left thumb stick
	float X = state->Gamepad.sThumbLX;
	float Y = state->Gamepad.sThumbLY;
	float magnitude = sqrt(X*X + Y*Y);
	float normalizedLX = X / magnitude;
	float normalizedLY = Y / magnitude;
	float LnormalizedMagnitude = -1;
	//Se verifica si el movimiento del análogo fue un movimiento significativo
	if (magnitude > LEFT_THUMB_DEADZONE)
	{
		if(magnitude > 32767) 
			magnitude = 32767;
		magnitude -= LEFT_THUMB_DEADZONE;
		LnormalizedMagnitude = magnitude / (32767 - LEFT_THUMB_DEADZONE);
		//Mueve la cámara según el movimiento del análogo izquierdo
		Cmr->MovePosition(normalizedLX, normalizedLY, LnormalizedMagnitude);
	}
	else 
	{
		Cmr->MoveIdle();
	}
	//right thumb stick
	X = state->Gamepad.sThumbRX;
	Y = state->Gamepad.sThumbRY;
	magnitude = sqrt(X*X + Y*Y);
	float normalizedRX = X / magnitude;
	float normalizedRY = Y / magnitude;
	float RnormalizedMagnitude = -1;
	//Se verifica si el movimiento del análogo fue un movimiento significativo
	if (magnitude > RIGHT_THUMB_DEADZONE)
	{
		if (magnitude > 32767)
			magnitude = 32767;
		magnitude -= RIGHT_THUMB_DEADZONE;
		RnormalizedMagnitude = magnitude / (32767 - RIGHT_THUMB_DEADZONE);
		//Mueve la dirección de la cámara según el análogo derecho.
		Cmr->MoveDirection(normalizedRX, normalizedRY, RnormalizedMagnitude);
	}
}
Ctrl360::~Ctrl360()
{

}
