/*
 * communicator.c
 *
 *  Created on: 07.05.2020
 *      Author: FKleb
 */

#include "communicator.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define DEFAULT_SERVER_IP_ADDRESS			("195.34.89.241")
#define DEFAULT_SERVER_PORT					(7)
#define DEFAULT_GAME_SERVER_IP_ADRESS		("52.57.105.0")
#define DEFAULT_GAME_SERVER_PORT			(44444)
#define SESSION_HEADER_LEN					9

uint16_t gSessionID = 0;
uint16_t gSequenceNr = 0;
uint32_t gNonce=0;
uint16_t gCcr=0;

static uint8_t secret[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x01};


static void cbNetworkReceive(uint8_t* pBuffer, uint32_t len);

static void cbNetworkReceive(uint8_t* pBuffer, uint32_t len) {
	printf("Packet received with len=%d.\r\n", len);

	for (int i=0; i<len; i++){
		printf("%02X  ",pBuffer[i]);
	}
	printf("\n");
	if (pBuffer[0]==0x04)
	{
		gNonce = read_msb4byte(&pBuffer[7]);
		Packet_t * crpacket = communicator_Sendcr();
	    network_send(crpacket->pBuffer,crpacket->len);
	    freepacket(crpacket);
	}

	if (pBuffer[0] == 0x10) {
		gSessionID = (pBuffer[3] << 8) | pBuffer[4];
		printf("SessoinID: %d", gSessionID);
	}

}

// Free ---------------------------------------------------------------------------------------

void freepacket(Packet_t * packet)
{
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
		if (!network_connect(DEFAULT_GAME_SERVER_IP_ADRESS, DEFAULT_GAME_SERVER_PORT)) {
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
	uint8_t * pBuffer = malloc( SESSION_HEADER_LEN);
	sessionFlags_t * header = &pBuffer[0];
	pBuffer[0] = 0;
	uint16_t hmac=0;
	header->heartbeat=1;
	write_msb2byte(&pBuffer[1],0);
	write_msb2byte(&pBuffer[3],gSessionID);
	write_msb2byte(&pBuffer[5],++gSequenceNr);
	write_msb2byte(&pBuffer[7],0);

	pheartbeat->pBuffer =&pBuffer[0];
	pheartbeat->len = SESSION_HEADER_LEN;
	hmac = calcHmac(pheartbeat->pBuffer,pheartbeat->len - 2);
	write_msb2byte(&pheartbeat->pBuffer[7],hmac);
	network_send(pheartbeat->pBuffer,pheartbeat->len);
	freepacket(pheartbeat);
}


void communicator_createSession(){
	uint8_t packet[9] = {0};
	sessionFlags_t* pHeader = (sessionFlags_t*)&packet[0];
	pHeader->version=0;
	pHeader->session_request=1;
	network_send(packet,9);
}

Packet_t * communicator_Sendcr()
{
	gCcr = calcCRC();
	Packet_t * ppacket = malloc(sizeof(Packet_t));
	uint8_t * pBuffer = malloc(SESSION_HEADER_LEN + 4);		// 4 == len
	sessionFlags_t * header = &pBuffer[0];
	pBuffer [0] = 0;
	header->session_challenge_response= 1;
	uint16_t len = 4;
	write_msb2byte(&pBuffer[1],len );
	write_msb2byte(&pBuffer[3],0);
	write_msb2byte(&pBuffer[5],0);
	write_msb2byte(&pBuffer[7],0);
	write_msb2byte(&pBuffer[9],gCcr);
	write_msb2byte(&pBuffer[11],0);

	ppacket->pBuffer =&pBuffer[0];
	ppacket->len = SESSION_HEADER_LEN + 4;   // HEADER_SIZE + len in der Funktion
	return ppacket;

}

//ceratpacket ------------------------------------------------------------------------------

Packet_t * creatpacket(uint32_t lenpayload,uint8_t type)
{
	if(type>6)
	{
		return NULL;
	}
	Packet_t * ppacket = malloc(sizeof(Packet_t));
	uint16_t len = SESSION_HEADER_LEN+lenpayload;
	uint8_t * pBuffer = malloc(len);
	sessionFlags_t * pHeader = &pBuffer[0];
	pHeader->version=0;
	if(type==1)  //createsession
	{
		pHeader->session_request=1;
	}
	else if(type==2) //session_challenge_response
	{
		pHeader->session_challenge_response= 1;

		write_msb2byte(&pBuffer[1],lenpayload);
		write_msb2byte(&pBuffer[3],0);
		write_msb2byte(&pBuffer[5],0);
		write_msb2byte(&pBuffer[7],0);
		write_msb2byte(&pBuffer[9],gCcr);
		write_msb2byte(&pBuffer[len-2],0);

		ppacket->pBuffer =&pBuffer[0];
		ppacket->len = SESSION_HEADER_LEN + lenpayload;
	}
	else if(type==3)// heartbeat
	{
		pHeader->heartbeat = 1;

		write_msb2byte(&pBuffer[1],lenpayload);
		write_msb2byte(&pBuffer[3],0);
		write_msb2byte(&pBuffer[5],0);
		write_msb2byte(&pBuffer[7],0);
		write_msb2byte(&pBuffer[9],0);
		write_msb2byte(&pBuffer[11],0);

		ppacket->pBuffer =&pBuffer[0];
		ppacket->len = SESSION_HEADER_LEN + lenpayload;
	}
	else if(type==4)
	{

	}
	else if(type==5)
	{

	}
	return ppacket;
}

// Paketgame_t------------------------------------------------------------------------------

PacketGame_t * registerPlayerPacket(uint8_t type, uint16_t len, char* playerName)
{
	// Packetlen = Header (fix 7 byte)+ 2 byte PL-Len + Type (1 Byte Player regist) + frei wählbare transaction id (2byte) + PL
	// PL = type (name) + len name (2byte) + n byte name
	uint8_t namelen= strlen(playerName);
	uint32_t sum= 7 + 2 + 1 + namelen;
	PacketGame_t * player = creatgamepacket(REGISTER_PLAYER, 0x2020 , sum);
	return player;
}


// creategamepacket-------------------------------------------------------------------------

PacketGame_t * creatgamepacket(CommandID_e type, uint16_t playerID, uint32_t sum)
{
	PacketGame_t * ppacket = malloc (sizeof(PacketGame_t));
	ppacket->len = sum;
	ppacket->pBuffer = malloc(ppacket->len);
	uint8_t * pBuffer = malloc(sum);
	sessionFlags_t * pHeader = &pBuffer[0];
	pHeader->version=0;
	if(type==REGISTER_PLAYER)
	{
		write_msb2byte(&pBuffer[1],sum-7); // - HEADER!!!
		write_msb2byte(&pBuffer[3],REGISTER_PLAYER);
		write_msb2byte(&pBuffer[5],playerID);

		ppacket->pBuffer =&pBuffer[0];

	}
	else if (type==PLAYER_CONTROLL)
	{
		//
	}
	else if (type==DROP_FOOD)
	{
		//
	}
	else if (type==CHAT_MESSAGE)
	{
		//
	}
	return ppacket;
}

// CALC ----------------------------------------------------------------------------------
uint16_t calcCRC()
{
	uint8_t cr[12] = { 0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x01 };
	write_msb4byte(&cr[0],gNonce);
	uint16_t hashvalue=CRC(&cr[0],12);
	write_msb2byte(&cr[0],hashvalue);
	uint16_t hashvalue2= CRC(&cr[0],2);
	gCcr=hashvalue2;
	 return hashvalue2;

}

uint16_t calcHmac(uint8_t* pbuff, uint32_t lenght)
{
	uint16_t sum = 8 + 2 + lenght; // anzahl der bytes für secret und cr
	uint8_t* phmac = malloc(sum);
	if (phmac == NULL){
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

//MSB R/W ------------------------------------------------------------------------------------

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
	uint32_t result = (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | (pBuf[3] << 0);
	return result;
}

// ---------------------------------------------------------------------------------------
