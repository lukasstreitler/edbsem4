#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*ReceivePacketHandler_t)(uint8_t* pBuffer, uint32_t len);

bool network_init(ReceivePacketHandler_t handler);
bool network_connect(const char* pIpAddress, uint16_t port);
bool network_send(uint8_t* pBuffer, int len);
void network_close(void);

#ifdef __cplusplus
}
#endif
