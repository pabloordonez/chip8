#include "gpu.h"

void gpu_reset(Gpu *gpu)
{
    memset(gpu, 0, sizeof(gpu->memory));
}

void gpu_clear_memory(Gpu *gpu)
{
    memset(gpu, 0, sizeof(Gpu));
}

bool gpu_draw_sprite(Gpu *gpu, u8 x, u8 y, u8 *memory, u8 from, u8 length)
{
    return false;
}