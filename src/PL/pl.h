/*
 * pl.h
 *
 *  Created on: 29.06.2020
 *      Author: lukas
 */

#ifndef SRC_PL_PL_H_
#define SRC_PL_PL_H_

#include"../network/network.h"
#include<stdbool.h>
#include"../communicator/communicator.h"
typedef enum {
	REGISTER_PLAYER = 0x01,
	PLAYER_CONTROLL = 0x10,
	DROP_FOOD = 0x20,
	REQUEST_PLAYER = 0x30,
	CHAT_MESSAGE = 0x40
} CommandID_e;

//typedef struct Packet {
//	uint8_t* pBuffer;
//	uint32_t len;
//} Packet_t;

Packet_t* registerPlayerPacket(uint16_t transactionID, char* playerName);
Packet_t * registermovemetpacket(bool up, bool down, bool left, bool right,
		uint16_t transactionID);
Packet_t * registerdroopfood(uint16_t transactionID);
Packet_t * registercreatemessage(char * message,uint16_t transactionID);

Packet_t * creategamepacket(CommandID_e type, uint16_t playerID, uint32_t sum);
#endif /* SRC_PL_PL_H_ */
