/*
 * pl.c
 *
 *  Created on: 29.06.2020
 *      Author: lukas
 */
#include "pl.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../network/network.h"
#include "../input/input_service.h"
#include "../communicator/communicator.h"

#define APP_HEADER_LEN		  				8

Packet_t * registerPlayerPacket(uint16_t transactionID, char* playerName) {
	uint8_t namelen = strlen(playerName);
	uint32_t sum = 7 + 1 + 2 + namelen;
	Packet_t * ppacket = creategamepacket(REGISTER_PLAYER, transactionID, sum);
	ppacket->pBuffer[7] = 0x01;
	write_msb2byte(&ppacket->pBuffer[8], namelen);
	memcpy(&ppacket->pBuffer[10], &playerName[0], namelen);
	return ppacket;
}
Packet_t * registermovemetpacket(bool up, bool down, bool left, bool right,
		uint16_t transactionID) {
	uint32_t len = APP_HEADER_LEN;
	Packet_t * ppacket = creategamepacket(PLAYER_CONTROLL, transactionID, len);
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
	Packet_t * ppacket = creategamepacket(DROP_FOOD, transactionID, len);
	ppacket->pBuffer[7] = 0;
	return ppacket;
}
Packet_t * registercreatemessage(char * message, uint16_t transactionID) {
	uint16_t messagelen = strlen(message);
	uint32_t sum = APP_HEADER_LEN - 1 + messagelen;
	Packet_t * ppacket = creategamepacket(CHAT_MESSAGE, transactionID, sum);
	memcpy(&ppacket->pBuffer[7], message, messagelen);
	return ppacket;

}
// creategamepacket-------------------------------------------------------------------------
Packet_t * creategamepacket(CommandID_e type, uint16_t transactionID,
		uint32_t sum) {
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
		write_msb2byte(&ppacket->pBuffer[3], CHAT_MESSAGE);
	}
	write_msb2byte(&ppacket->pBuffer[1], sum - 7);
	write_msb2byte(&ppacket->pBuffer[5], transactionID);
	return ppacket;
}
