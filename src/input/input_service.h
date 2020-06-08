#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef uint32_t InputKeyMask_t;

#define INPUT_KEY_MASK_NONE					(0)
#define INPUT_KEY_MASK_KEY_UP				(0b00000001)
#define INPUT_KEY_MASK_KEY_DOWN				(0b00000010)
#define INPUT_KEY_MASK_KEY_LEFT				(0b00000100)
#define INPUT_KEY_MASK_KEY_RIGHT			(0b00001000)
#define INPUT_KEY_MASK_KEY_ESC				(0b00010000)
#define INPUT_KEY_MASK_KEY_SPACE			(0b00100000)

typedef void(*InputEventHandler)(InputKeyMask_t);

bool input_service_init(InputEventHandler handler);

#ifdef __cplusplus
}
#endif
