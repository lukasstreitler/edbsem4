#include "input_service.h"
#include <windows.h>

static InputEventHandler gEventHandler;
static HANDLE gInputThreadHandle;

DWORD WINAPI inputHandleThread(void* data);

#ifndef INPUT_SERVICE_UPDATE_RATE_Hz
#define INPUT_SERVICE_UPDATE_RATE_Hz						(100)
#endif /* INPUT_SERVICE_UPDATE_RATE_Hz */

bool input_service_init(InputEventHandler handler) {
	gEventHandler = handler;
	gInputThreadHandle = CreateThread(NULL, 0, inputHandleThread, NULL, 0, NULL);
	return (gInputThreadHandle != NULL);
}

DWORD WINAPI inputHandleThread(void* data) {
	InputKeyMask_t lastMask = INPUT_KEY_MASK_NONE;
	while (true) {
		InputKeyMask_t currMask = INPUT_KEY_MASK_NONE;
		currMask |= (GetAsyncKeyState(VK_UP)) ? INPUT_KEY_MASK_KEY_UP : 0;
		currMask |= (GetAsyncKeyState(VK_DOWN)) ? INPUT_KEY_MASK_KEY_DOWN : 0;
		currMask |= (GetAsyncKeyState(VK_LEFT)) ? INPUT_KEY_MASK_KEY_LEFT : 0;
		currMask |= (GetAsyncKeyState(VK_RIGHT)) ? INPUT_KEY_MASK_KEY_RIGHT : 0;
		currMask |= (GetAsyncKeyState(VK_ESCAPE)) ? INPUT_KEY_MASK_KEY_ESC : 0;
		currMask |= (GetAsyncKeyState(VK_SPACE)) ? INPUT_KEY_MASK_KEY_SPACE : 0;

		if (currMask != lastMask && gEventHandler) {
			gEventHandler(currMask);
		}
		lastMask = currMask;
		Sleep(1000 / INPUT_SERVICE_UPDATE_RATE_Hz); 
	}
}
