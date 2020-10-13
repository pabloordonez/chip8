#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "types.h"

/**
 * Defines a keyboard device.
 * The chip-8 keyboard device contains 16 keys.
 */
typedef struct Keyboard
{
    u16 memory;
} Keyboard;

#define NOT_KEY_PRESSED -1

bool keyboard_is_any_key_pressed(Keyboard *keyboard);

bool keyboard_is_key_pressed(Keyboard *keyboard, u8 key);

i8 keyboard_get_key_pressed_index(Keyboard *keyboard);

#endif /* __KEYBOARD_H__ */