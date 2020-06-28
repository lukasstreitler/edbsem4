#include "input/input_service.h"
#include "windows.h"
#include <stdio.h>
#include "communicator/communicator.h"
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

static bool gRunning = true;
static void cbInputHandler(InputKeyMask_t);
static uint16_t playerID = 0x2020;
static char* playername = "Lukas";
//IP 80.110.104.6
int main(int argc, char** argv) {
//test
	setbuf(stdout, NULL);
	//uint8_t test[3] = { 0, 1, 2 };

	printf("%d\n\r", sizeof(sessionFlags_t));

	input_service_init(cbInputHandler);
	communicator_connect(GAME_SERVER);
	communicator_createSession();
	communicator_heartbeat();
	communicator_sendcompplayerreg(playerID, playername);
	uint32_t sekunden = time(NULL);
	uint32_t lastheartebeat = sekunden;

	while (gRunning) {
		sekunden = time(NULL);
		if ((sekunden - lastheartebeat) > 10) {
			communicator_heartbeat();
			printf("Sending Heartbeat\n");
			lastheartebeat = sekunden;

		}

		Sleep(20);
	}
	printf("Exiting...");
	return 0;
}

static void cbInputHandler(InputKeyMask_t m) {
	printf("Input event: %d.\r\n", m);
	if (m == INPUT_KEY_MASK_KEY_ESC) {
		gRunning = false;
	}

	if (m & INPUT_KEY_MASK_KEY_SPACE) {
		communicator_drop(playerID);
	}
	bool up = (m & INPUT_KEY_MASK_KEY_UP);
	bool down = (m & INPUT_KEY_MASK_KEY_DOWN);
	bool left = (m & INPUT_KEY_MASK_KEY_LEFT);
	bool right = (m & INPUT_KEY_MASK_KEY_RIGHT);
	if (!(((up == true) && (down == true))
			|| ((left == true) && (right == true)))) {
		communicator_movement(up, down, left , right, playerID);
	}
}
