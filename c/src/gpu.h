#ifndef __GPU_H__
#define __GPU_H__

#include <string.h>
#include "types.h"

/**
 * Defines a gpu device.
 * The chip-8 device contains a 64x32 black and white display.
 */
typedef struct Gpu
{
    u8 memory[256];
} Gpu;

void gpu_reset(Gpu *gpu);

void gpu_clear_memory(Gpu *gpu);

bool gpu_draw_sprite(Gpu *gpu, u8 x, u8 y, const u8 *memory, u8 from, u8 length);

#endif /*__GPU_H__*/