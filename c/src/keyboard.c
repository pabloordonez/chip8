#include "keyboard.h"

void keyboard_reset(Keyboard *keyboard)
{
    keyboard->memory = 0;
}

bool keyboard_is_any_key_pressed(const Keyboard *keyboard)
{
    return keyboard->memory != 0;
}

bool keyboard_is_key_pressed(const Keyboard *keyboard, u8 key)
{
    return ((keyboard->memory >> key) & 0x01) != 0;
}

i8 keyboard_get_key_pressed_index(const Keyboard *keyboard)
{
    u16 keys = keyboard->memory;

    for (u8 i = 0; i < 16; i++)
    {
        if (((keys >> i) & 0x0001) != 0)
        {
            return i;
        }
    }

    return NOT_KEY_PRESSED;
}
