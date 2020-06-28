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

//ENUM-----------------------------------------------------------------------------------

typedef enum {
	TEST_SERVER, MAIN_SERVER, GAME_SERVER
} Server_e;

typedef enum {
	REGISTER_PLAYER = 0x01,
	PLAYER_CONTROLL = 0x10,
	DROP_FOOD = 0x20,
	REQUEST_PLAYER = 0x30,
	CHAT_MESSAGE = 0x40
} CommandID_e;

//struct-----------------------------------------------------------------------------------

typedef struct {
	uint8_t heartbeat :1;
	uint8_t session_request :1;
	uint8_t session_challenge :1;
	uint8_t session_challenge_response :1;
	uint8_t session_result :1;
	uint8_t session_invalidate :1;
	uint8_t version :2;

} sessionFlags_t;

typedef struct Packet {
	uint8_t* pBuffer;
	uint32_t len;
} Packet_t;

//func -----------------------------------------------------------------------------------------

void communicator_heartbeat();
int communicator_connect(Server_e);
void communicator_createSession();
void write_msb2byte(uint8_t* padd, uint16_t val);
void write_msb4byte(uint8_t* padd, uint32_t val);
uint16_t calcCRC();
uint16_t CRC(uint8_t* padd, uint32_t len);
uint32_t read_msb4byte(uint8_t* pBuf);
uint16_t read_msb2byte(uint8_t* pBuf);
Packet_t * communicator_Sendcr();
void freepacket(Packet_t * packet);
uint16_t calcHmac(uint8_t* pbuff, uint32_t lenght);
Packet_t* registerPlayerPacket(uint16_t transactionID, char* playerName);
Packet_t * creatgamepacket(CommandID_e type, uint16_t playerID, uint32_t sum);
void Communicator_sendappmessage(uint8_t * pl, uint32_t pllen);
void communicator_sendcompplayerreg(uint16_t transactionID, char* playerName);
Packet_t * registermovemetpacket(bool up, bool down, bool left, bool right,
		uint16_t transactionID);
Packet_t * registerdroopfood(uint16_t transactionID);
void communicator_drop(uint16_t transactionID);

#endif /* SRC_COMMUNICATOR_COMMUNICATOR_H_ */
