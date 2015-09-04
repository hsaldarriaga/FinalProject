#pragma once
#include "Camera.h"
#include <Windows.h>
#include <Xinput.h>

#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	#pragma comment(lib, "Xinput.lib")
#else 
	#pragma comment(lib, "Xinput9_1_0.lib")
#endif

#define LEFT_THUMB_DEADZONE  13000
#define RIGHT_THUMB_DEADZONE 13000
#define TRIGGER_THRESHOLD    30

class Ctrl360
{
public:
	Ctrl360(Camera*);
	void Init();
	void CaptureUserInteractions();
	~Ctrl360();
private:
	bool Connect();
	void ManageThumbs(XINPUT_STATE* state);
	XINPUT_STATE oldstate;
	DWORD control_index;
	Camera* Cmr;
};

