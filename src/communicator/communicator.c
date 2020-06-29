/*
 * communicator.c
 *
 *  Created on: 07.05.2020
 *      Author: LukasStreitler
 */
//#include------------------------------------------------------------------------------------
#include "communicator.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../network/network.h"
#include "../input/input_service.h"
//define---------------------------------------------------------------------------------------
#define DEFAULT_SERVER_IP_ADDRESS			("195.34.89.241")
#define DEFAULT_SERVER_PORT					(7)
#define DEFAULT_GAME_SERVER_IP_ADRESS		("52.57.105.0")
#define DEFAULT_GAME_SERVER_PORT			(44444)
#define SESSION_HEADER_LEN					9
#define APP_HEADER_LEN		  				8
//globel---------------------------------------------------------------------------------------
uint16_t gSessionID = 0;
uint16_t gSequenceNr = 0;
uint32_t gNonce = 0;
uint16_t gCcr = 0;
uint16_t gstate = 0;
// static--------------------------------------------------------------------------------------
static uint8_t secret[8] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x01 };
static void cbNetworkReceive(uint8_t* pBuffer, uint32_t len);
//cbNetworkReceive-----------------------------------------------------------------------------
static void cbNetworkReceive(uint8_t* pBuffer, uint32_t len) {
	printf("Packet received with len=%d.\r\n", len);

	for (int i = 0; i < len; i++) {
		printf("%02X  ", pBuffer[i]);
	}
	printf("\n");
	if (pBuffer[0] == 0x04) {
		gNonce = read_msb4byte(&pBuffer[7]);
		Packet_t * crpacket = communicator_Sendcr();
		network_send(crpacket->pBuffer, crpacket->len);
		gstate = 1;
		freepacket(crpacket);
	}

	if (pBuffer[0] == 0x10) {
		gSessionID = (pBuffer[3] << 8) | pBuffer[4];
		printf("SessoinID: %d\n", gSessionID);
	}

}
// Free ---------------------------------------------------------------------------------------
void freepacket(Packet_t * packet) {
	free(packet->pBuffer);
	free(packet);
}
// communicator--------------------------------------------------------------------------------
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
	case GAME_SERVER:
		if (!network_connect(DEFAULT_GAME_SERVER_IP_ADRESS,
		DEFAULT_GAME_SERVER_PORT)) {
			return -2;
		}
		break;
	default:
		return -3;
		break;
	}

	return 0;
}
void communicator_heartbeat() {
	Packet_t * pheartbeat = malloc(sizeof(Packet_t));
	pheartbeat->len = SESSION_HEADER_LEN;
	pheartbeat->pBuffer = malloc(pheartbeat->len);
	memset(pheartbeat->pBuffer, 0x00, pheartbeat->len);
	sessionFlags_t * header = (sessionFlags_t*) pheartbeat->pBuffer;
	header->heartbeat = 1;
	write_msb2byte(&pheartbeat->pBuffer[1], 0);
	write_msb2byte(&pheartbeat->pBuffer[3], gSessionID);
	write_msb2byte(&pheartbeat->pBuffer[5], ++gSequenceNr);
	uint16_t hmac = calcHmac(pheartbeat->pBuffer, pheartbeat->len - 2);
	write_msb2byte(&pheartbeat->pBuffer[7], hmac);
	network_send(pheartbeat->pBuffer, pheartbeat->len);
	printf("\n%d\n", gSessionID);
	freepacket(pheartbeat);

}
void communicator_createSession() {
	uint8_t packet[9] = { 0 };
	sessionFlags_t* pHeader = (sessionFlags_t*) &packet[0];
	pHeader->version = 0;
	pHeader->session_request = 1;
	network_send(packet, 9);
	while (!gSessionID) {
		Sleep(10);
	}
}
Packet_t * communicator_Sendcr()  //ausbessern falls zeit
{
	gCcr = calcCRC();
	Packet_t * ppacket = malloc(sizeof(Packet_t));
	uint8_t * pBuffer = malloc(SESSION_HEADER_LEN + 4);		// 4 == len
	sessionFlags_t * header = &pBuffer[0];
	pBuffer[0] = 0;
	header->session_challenge_response = 1;
	uint16_t len = 4;
	write_msb2byte(&pBuffer[1], len);
	write_msb2byte(&pBuffer[3], 0);
	write_msb2byte(&pBuffer[5], 0);
	write_msb2byte(&pBuffer[7], 0);
	write_msb2byte(&pBuffer[9], gCcr);
	write_msb2byte(&pBuffer[11], 0);

	ppacket->pBuffer = &pBuffer[0];
	ppacket->len = SESSION_HEADER_LEN + 4;  // HEADER_SIZE + len in der Funktion
	return ppacket;

}
void Communicator_sendappmessage(uint8_t * pl, uint32_t pllen) {

	Packet_t * pplayerreg = malloc(sizeof(Packet_t));
	pplayerreg->len = SESSION_HEADER_LEN + pllen;
	pplayerreg->pBuffer = malloc(pplayerreg->len);
	memset(pplayerreg->pBuffer, 0x00, pplayerreg->len);
	write_msb2byte(&pplayerreg->pBuffer[1], pllen);
	write_msb2byte(&pplayerreg->pBuffer[3], gSessionID);
	write_msb2byte(&pplayerreg->pBuffer[5], ++gSequenceNr);
	memcpy(&pplayerreg->pBuffer[7], &pl[0], pllen);
	uint16_t hmac = calcHmac(pplayerreg->pBuffer, pplayerreg->len - 2);
	write_msb2byte(&pplayerreg->pBuffer[SESSION_HEADER_LEN - 2 + pllen], hmac);
	network_send(pplayerreg->pBuffer, pplayerreg->len);
	freepacket(pplayerreg);

}
void communicator_sendcompplayerreg(uint16_t transactionID, char* playerName) {
	Packet_t * pPlayerRegsiterPacket = registerPlayerPacket(transactionID,
			playerName);
	Communicator_sendappmessage(pPlayerRegsiterPacket->pBuffer,
			pPlayerRegsiterPacket->len);

}
void communicator_movement(bool up, bool down, bool left, bool right,
		uint16_t transactionID) {
	Packet_t * pmove = registermovemetpacket(up, right, down, left,
			transactionID);
	Communicator_sendappmessage(pmove->pBuffer, pmove->len);
}
void communicator_drop(uint16_t transactionID) {
	Packet_t * pdropfood = registerdroopfood(transactionID);
	Communicator_sendappmessage(pdropfood->pBuffer, pdropfood->len);
}
void communicator_sendmessage(char * message,uint16_t transactionID) {
	Packet_t * pmessage = registercreatemessage(message,transactionID);
	Communicator_sendappmessage(pmessage->pBuffer, pmessage->len);
}
//ceratepacket ------------------------------------------------------------------------------
Packet_t * creatpacket(uint32_t lenpayload, uint8_t type) //erweitern falls zeit

{
	if (type > 6) {
		return NULL;
	}
	Packet_t * ppacket = malloc(sizeof(Packet_t));
	uint16_t len = SESSION_HEADER_LEN + lenpayload;
	uint8_t * pBuffer = malloc(len);
	sessionFlags_t * pHeader = &pBuffer[0];
	pHeader->version = 0;

	if (type == 1)  //createsession
			{
		pHeader->session_request = 1;
	} else if (type == 2) //session_challenge_response
			{
		pHeader->session_challenge_response = 1;

		write_msb2byte(&pBuffer[1], lenpayload);
		write_msb2byte(&pBuffer[3], 0);
		write_msb2byte(&pBuffer[5], 0);
		write_msb2byte(&pBuffer[7], 0);
		write_msb2byte(&pBuffer[9], gCcr);
		write_msb2byte(&pBuffer[len - 2], 0);

		ppacket->pBuffer = &pBuffer[0];
		ppacket->len = SESSION_HEADER_LEN + lenpayload;
	} else if (type == 3) // heartbeat
			{
		pHeader->heartbeat = 1;

		write_msb2byte(&pBuffer[1], lenpayload);
		write_msb2byte(&pBuffer[3], 0);
		write_msb2byte(&pBuffer[5], 0);
		write_msb2byte(&pBuffer[7], 0);
		write_msb2byte(&pBuffer[9], 0);
		write_msb2byte(&pBuffer[11], 0);

		ppacket->pBuffer = &pBuffer[0];
		ppacket->len = SESSION_HEADER_LEN + lenpayload;
	} else if (type == 4) {

	} else if (type == 5) {

	}
	return ppacket;
}
// Paketgame_t-------------------------------------------------------------------------------
Packet_t * registerPlayerPacket(uint16_t transactionID, char* playerName) {
	uint8_t namelen = strlen(playerName);
	uint32_t sum = 7 + 1 + 2 + namelen;
	Packet_t * ppacket = creategamepacket(REGISTER_PLAYER,transactionID,sum);
	ppacket->pBuffer[7] = 0x01;
	write_msb2byte(&ppacket->pBuffer[8], namelen);
	memcpy(&ppacket->pBuffer[10], &playerName[0], namelen);
	return ppacket;
}
Packet_t * registermovemetpacket(bool up, bool down, bool left, bool right,
		uint16_t transactionID) {
	uint32_t len = APP_HEADER_LEN;
	Packet_t * ppacket = creategamepacket(PLAYER_CONTROLL,transactionID,len);
	ppacket->pBuffer[7] = 0;

	if (up == true) {
		ppacket->pBuffer[7] = (ppacket->pBuffer[7] | INPUT_KEY_MASK_KEY_UP);
	}
	if (right == true) {
		ppacket->pBuffer[7] = (ppacket->pBuffer[7] | INPUT_KEY_MASK_KEY_RIGHT);
	}
	if (down == true) {
		ppacket->pBuffer[7] = (ppacket->pBuffer[7] | INPUT_KEY_MASK_KEY_DOWN);
	}
	if (left == true) {
		ppacket->pBuffer[7] = (ppacket->pBuffer[7] | INPUT_KEY_MASK_KEY_LEFT);
	}

	return ppacket;
}
Packet_t * registerdroopfood(uint16_t transactionID) {
	uint32_t len = APP_HEADER_LEN;
	Packet_t * ppacket = creategamepacket(DROP_FOOD,transactionID,len);
	ppacket->pBuffer[7] = 0;
	return ppacket;
}
Packet_t * registercreatemessage(char * message,uint16_t transactionID){
	uint16_t messagelen= strlen(message);
	uint32_t sum = APP_HEADER_LEN - 1 + messagelen;
	Packet_t * ppacket = creategamepacket(CHAT_MESSAGE,transactionID,sum);
	memcpy(&ppacket->pBuffer[7],message,messagelen);
	return ppacket;

}
// creategamepacket-------------------------------------------------------------------------
Packet_t * creategamepacket(CommandID_e type, uint16_t transactionID,
		uint32_t sum)
{
	Packet_t * ppacket = malloc(sizeof(Packet_t));
	ppacket->len = sum;
	ppacket->pBuffer = malloc(ppacket->len);
	memset(ppacket->pBuffer, 0x00, ppacket->len);
	ppacket->pBuffer[0] = 0x01;
	if (type == REGISTER_PLAYER) {
		write_msb2byte(&ppacket->pBuffer[3], REGISTER_PLAYER);

	} else if (type == PLAYER_CONTROLL) {
		write_msb2byte(&ppacket->pBuffer[3], PLAYER_CONTROLL);
	} else if (type == DROP_FOOD) {
		write_msb2byte(&ppacket->pBuffer[3], DROP_FOOD);
	} else if (type == CHAT_MESSAGE) {
		write_msb2byte(&ppacket->pBuffer[3],CHAT_MESSAGE);
	}
	write_msb2byte(&ppacket->pBuffer[1], sum - 7);
	write_msb2byte(&ppacket->pBuffer[5], transactionID);
	return ppacket;
}
// CALC ------------------------------------------------------------------------------------
uint16_t calcCRC() {
	uint8_t cr[12] = { 0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
			0xFF, 0x00, 0x01 };
	write_msb4byte(&cr[0], gNonce);
	uint16_t hashvalue = CRC(&cr[0], 12);
	write_msb2byte(&cr[0], hashvalue);
	uint16_t hashvalue2 = CRC(&cr[0], 2);
	gCcr = hashvalue2;
	return hashvalue2;

}
uint16_t calcHmac(uint8_t* pbuff, uint32_t lenght) {
	uint16_t sum = 8 + 2 + lenght; // anzahl der bytes für secret und cr
	uint8_t* phmac = malloc(sum);
	if (phmac == NULL) {
		return 0;
	}
	write_msb2byte(&phmac[0], gCcr);
	memcpy(&phmac[2], secret, 8);
	memcpy(&phmac[10], pbuff, lenght);
	uint16_t hash = CRC(phmac, sum);
	write_msb2byte(&phmac[0], hash);
	uint16_t hmac = CRC(phmac, 2);
	return hmac;

}
uint16_t CRC(uint8_t* padd, uint32_t len) {
	uint16_t calkcrc = 0xFFFF;
	for (int i = 0; i < len; i++) {
		calkcrc = calkcrc ^ padd[i];
		for (int j = 0; j < 8; j++) {
			if (calkcrc & 0b1) {
				calkcrc = calkcrc >> 1;
				calkcrc = calkcrc ^ 0xA001;
			} else {
				calkcrc = calkcrc >> 1;
			}
		}
	}
	return calkcrc;
}
//MSB R/W -----------------------------------------------------------------------------------
void write_msb2byte(uint8_t* padd, uint16_t val) {
	padd[0] = (val & 0xFF00) >> 8;
	padd[1] = (val & 0xFF);
}
void write_msb4byte(uint8_t* padd, uint32_t val) {
	padd[0] = (val & 0xFF000000) >> 24;
	padd[1] = (val & 0xFF0000) >> 16;
	padd[2] = (val & 0xFF00) >> 8;
	padd[3] = (val & 0xFF);
}
uint16_t read_msb2byte(uint8_t* pBuf) {
	uint16_t result = (pBuf[0] << 8) | (pBuf[1]);
	return result;
}
uint32_t read_msb4byte(uint8_t* pBuf) {
	uint32_t result = (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8)
			| (pBuf[3] << 0);
	return result;
}
// ------------------------------------------------------------------------------------------
