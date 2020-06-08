/*
 * communicator.c
 *
 *  Created on: 07.05.2020
 *      Author: FKleb
 */

#include "communicator.h"
#include <stdint.h>
#include <stdio.h>

#define DEFAULT_SERVER_IP_ADDRESS					("195.34.89.241")
#define DEFAULT_SERVER_PORT							(7)

static void cbNetworkReceive(uint8_t* pBuffer, uint32_t len);

static void cbNetworkReceive(uint8_t* pBuffer, uint32_t len) {
	printf("Packet received with len=%d.\r\n", len);
}

int communicator_connect(Server_e server) {
	if (!network_init(cbNetworkReceive)) {
		return -1;
	}
	switch (server) {
	case MAIN_SERVER:
		return -3;
		break;
	case TEST_SERVER:
		if (!network_connect(DEFAULT_SERVER_IP_ADDRESS, DEFAULT_SERVER_PORT)) {
			return -2;
		}
		break;
	default:
		return -3;
		break;
	}

	return 0;
}

packethead_t* setPacketHeader(uint8_t* packet){
	return (packethead_t*)packet;
}

void setHeaderVersion(packethead_t* header, int version){
	header->version = version;
}
void setHeaderExit(packethead_t* header, bool wantExit){
	header->exit = wantExit;
}
void setHeaderChallenge(packethead_t* header, bool wantChallenge){
	header->challenge = wantChallenge;
}
void setHeaderRequest(packethead_t* header, bool wantRequest){
	header->request = wantRequest;
}
void setHeaderHeartbeat(packethead_t* header, bool heartbeat){
	header->heartbeat = heartbeat;
}

void communicator_createSession(){
	uint8_t packet[2] = {0};
	//packethead_t* header = (packethead_t*)&packet[0];
	packethead_t* header = setPacketHeader(packet);
	setHeaderVersion(header, 0);
	setHeaderExit(header, 0);
	setHeaderChallenge(header, 0);
	setHeaderRequest(header, 1);
	setHeaderHeartbeat(header, 0);
	network_send(packet, 2);

}

