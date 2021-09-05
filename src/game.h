#ifndef GAME_DEF
#define GAME_DEF

#include <stdint.h>

#include "window.h"

#define CAMERA_FOV (60.0f * (PI / 180.0f))
#define CAMERA_FAR 70
#define CAMERA_NEAR 0.1
#define CAMERA_ASPECT (WINDOW_INIT_WIDTH/(float)WINDOW_INIT_HEIGHT)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

float GetDeltaTime(void);

#endif