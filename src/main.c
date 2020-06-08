
#include "input/input_service.h"
#include "windows.h"
#include <stdio.h>
#include "communicator/communicator.h"

static bool gRunning = true;
static void cbInputHandler(InputKeyMask_t);

int main(int argc, char** argv) {

	setbuf(stdout, NULL);
	uint8_t test[3] = { 0, 1, 2 };

	printf("%d\n\r",sizeof(packethead_t));

	input_service_init(cbInputHandler);
	if (communicator_connect(TEST_SERVER) != 0){
		printf("Fail..\r\n");
		return -1;
	}

	while(gRunning) {
		//network_send(test, 3);
		Sleep(1000);
	}
	printf("Exiting...");
	return 0;
}

static void cbInputHandler(InputKeyMask_t m) {
	printf("Input event: %d.\r\n", m);
	if (m == INPUT_KEY_MASK_KEY_ESC) {
		gRunning = false;
   	}

	if(m & INPUT_KEY_MASK_KEY_SPACE){
		uint8_t test[3] = { 0, 1, 2 };
		network_send(test, 3);
		printf("Packet sent.\r\n");
	}
}
