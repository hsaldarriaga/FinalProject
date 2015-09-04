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

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(i, &oldstate);

		if (dwResult == ERROR_SUCCESS)
		{
			XINPUT_CAPABILITIES caps;
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
	if (dwResult == ERROR_SUCCESS)
	{
		//Button actions
		if (state.dwPacketNumber != oldstate.dwPacketNumber && state.Gamepad.wButtons != 0)
		{
			
		}
		//Analog actions
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
	if (magnitude > LEFT_THUMB_DEADZONE)
	{
		if(magnitude > 32767) 
			magnitude = 32767;
		magnitude -= LEFT_THUMB_DEADZONE;
		LnormalizedMagnitude = magnitude / (32767 - LEFT_THUMB_DEADZONE);
		Cmr->MovePosition(normalizedLX, normalizedLY, LnormalizedMagnitude);
	}
	//right thumb stick
	X = state->Gamepad.sThumbRX;
	Y = state->Gamepad.sThumbRY;
	magnitude = sqrt(X*X + Y*Y);
	float normalizedRX = X / magnitude;
	float normalizedRY = Y / magnitude;
	float RnormalizedMagnitude = -1;
	if (magnitude > RIGHT_THUMB_DEADZONE)
	{
		if (magnitude > 32767)
			magnitude = 32767;
		magnitude -= RIGHT_THUMB_DEADZONE;
		RnormalizedMagnitude = magnitude / (32767 - RIGHT_THUMB_DEADZONE);
		Cmr->MoveDirection(normalizedRX, normalizedRY, RnormalizedMagnitude);
	}
}
Ctrl360::~Ctrl360()
{

}
