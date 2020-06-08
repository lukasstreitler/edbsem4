#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "network.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

static ReceivePacketHandler_t gReceiveHandler;
static SOCKET gSocket;
static struct sockaddr_in gServerInfo;
static HANDLE gReceiveThreadHandle;

static bool gConnected;

bool hasSocketError(SOCKET socket);
DWORD WINAPI receiveThread(void* data);

bool network_init(ReceivePacketHandler_t handler) {
	gReceiveHandler = handler;
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return result == 0;
}

bool network_connect(const char* pIpAddress, uint16_t port) {
	gServerInfo.sin_family = AF_INET;
	gServerInfo.sin_addr.s_addr = INADDR_ANY;
	gServerInfo.sin_port = htons(port);
	gServerInfo.sin_addr.S_un.S_addr = inet_addr(pIpAddress);
	gSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (!(gConnected = !hasSocketError(gSocket))) {
		network_close();
		return false;
	}
	
	gReceiveThreadHandle = CreateThread(NULL, 0, receiveThread, NULL, 0, NULL);
	gConnected = gReceiveThreadHandle;
	if (!gConnected) {
		network_close();
		return false;
	}
	return gConnected;
}

bool network_send(uint8_t* pBuffer, int len) {
	if (!gConnected) {
		return false;
	}
	int result = sendto(gSocket, (char*)pBuffer, len, 0, (const struct sockaddr*)&gServerInfo, sizeof(struct sockaddr_in));
	return result == len;
}

void network_close() {
	closesocket(gSocket);
	gConnected = false;
}

DWORD WINAPI receiveThread(void* data) {
	uint8_t pBuffer[1024];
	while (gConnected) {
		int result = recvfrom(gSocket, (char*)pBuffer, sizeof(pBuffer), 0, NULL, 0);
		if (result > 0 && gReceiveHandler) {
			gReceiveHandler(pBuffer, result);
		}
	}
	return 0;
}

bool hasSocketError(SOCKET socket) {
	char error_code;
	int error_code_size = sizeof(error_code);
	getsockopt(socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
	return error_code != 0;
}
