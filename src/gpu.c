#include "gpu.h"

u8 gpu_get_pixel(Gpu *gpu, u8 x, u8 y)
{
    return gpu->memory[y * GPU_SCREEN_WIDTH + x];
}

void gpu_set_pixel(Gpu *gpu, u8 x, u8 y, u8 value)
{
    gpu->memory[y * GPU_SCREEN_WIDTH + x] = value;
}

void gpu_reset(Gpu *gpu)
{
    memset(gpu->memory, (u8)0, sizeof(gpu->memory));
}

bool gpu_draw_sprite(Gpu *gpu, u8 x, u8 y, const u8 *memory, u16 index_from, u8 length)
{
    bool collision = false;
    for (u16 rows = 0; rows < length; rows++)
    {
        u8 sprite = memory[index_from + rows];

        for (u8 bit = 0; bit < 8; bit++)
        {
            u8 new_value = (sprite >> (7 - bit)) & 0x01;

            u8 px = (x + bit) % GPU_SCREEN_WIDTH;
            u8 py = (y + rows) % GPU_SCREEN_HEIGHT;
            u8 old_value = gpu_get_pixel(gpu, px, py);

            collision = collision || (old_value == 1 && new_value == 1);
            gpu_set_pixel(gpu, px, py, old_value ^ new_value);
        }
    }

    return collision;
}