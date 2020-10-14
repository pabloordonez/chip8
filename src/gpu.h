#ifndef __GPU_H__
#define __GPU_H__

#include <string.h>
#include <math.h>
#include "types.h"

#define GPU_SCREEN_WIDTH 64
#define GPU_SCREEN_HEIGHT 32

/**
 * Defines a gpu device.
 * The chip-8 device contains a 64x32 black and white display.
 */
typedef struct Gpu
{
    u8 memory[GPU_SCREEN_WIDTH * GPU_SCREEN_HEIGHT];
} Gpu;

inline u8 gpu_get_pixel(Gpu *gpu, u8 x, u8 y);

inline void gpu_set_pixel(Gpu *gpu, u8 x, u8 y, u8 value);

void gpu_reset(Gpu *gpu);

bool gpu_draw_sprite(Gpu *gpu, u8 x, u8 y, const u8 *memory, u16 index_from, u8 length);

#endif /*__GPU_H__*/