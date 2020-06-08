/*
 * communicator.h
 *
 *  Created on: 07.05.2020
 *      Author: FKleb
 */

#ifndef SRC_COMMUNICATOR_COMMUNICATOR_H_
#define SRC_COMMUNICATOR_COMMUNICATOR_H_

#include"../network/network.h"
#include<stdbool.h>

typedef enum{
	TEST_SERVER,
	MAIN_SERVER
}Server_e;

typedef struct {
	uint8_t version : 2;
	uint8_t rfu : 1;
	uint8_t exit : 1;
	uint8_t challenge: 1;
	uint8_t request: 1;
	uint8_t heartbeat: 1;
}packethead_t;

int communicator_connect(Server_e);
/*void communicator_createSession();
void setHeaderVersion(packethead_t* header, int version);
void setHeaderExit(packethead_t* header, bool wantExit);
void setHeaderChallenge(packethead_t* header, bool wantChallenge);
void setHeaderRequest(packethead_t* header, bool wantRequest);
void setHeaderHeartbeat(packethead_t* header, bool heartbeat);
*/
#endif /* SRC_COMMUNICATOR_COMMUNICATOR_H_ */
