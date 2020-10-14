# Chip 8 Interpreter
A chip 8 interpreter made in c using [raylib](https://www.raylib.com/) for graphics,
inputs, and not yet implemented: sounds. This is a work in progress, part of a hobby exploration
and not intended as a final product or application. Is the warm up before moving to bigger projects
like a nes emulator.

Be aware that this is my first c project after probably more than 10 years, so i'm not expecting the
code to be perfect nor follow every convention or best practice out there. Said that, I think the code
is pretty lean and self explanatory, although I'll try to comment more.

## Known Issues
The chip 8 documentation suggest instructions should be padded to be properly aligned, but this doesn't
seem to be true for all roms out there. Some roms like the INVADERS, jumps to an odd address, and starts,
moving on odd addresses from that point. This currently breaks the disassembly process that assumes every
instruction will be aligned. I need to make a smarter disassembly process, right now it's just a cycling
with a fetch and decode only.

Need to improve the rom reading process to be more efficient, I just drop the easiest code there, but I should
reduce the IO operations and read to a buffer instead.

## Missing Features
A lot, a lot a lot a lot. Sound is missing, I need to verify the clock and how the sound and delay timer work,
some games seem to respond weird to keyboard interruptions (see Fx0A - LD Vx, K).
And there are plenty other things I want to implement or test.

## Instruction Set
Currently the application supports only the standard instruction set, with the standard memory addresses and screen resolution.
Here's a list of all supported instructions (for a more in depth look at each instruction please read [here](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)):

```
00E0 - CLS
00EE - RET

0nnn - SYS addr
1nnn - JP addr
2nnn - CALL addr
Annn - LD I, addr
Bnnn - JP V0, addr

3xkk - SE Vx, byte
4xkk - SNE Vx, byte
6xkk - LD Vx, byte
7xkk - ADD Vx, byte
Cxkk - RND Vx, byte

5xy0 - SE Vx, Vy
8xy0 - LD Vx, Vy
8xy1 - OR Vx, Vy
8xy2 - AND Vx, Vy
8xy3 - XOR Vx, Vy
8xy4 - ADD Vx, Vy
8xy5 - SUB Vx, Vy
8xy6 - SHR Vx {, Vy}
8xy7 - SUBN Vx, Vy
8xyE - SHL Vx {, Vy}
9xy0 - SNE Vx, Vy
Dxyn - DRW Vx, Vy, nibble

Ex9E - SKP Vx
ExA1 - SKNP Vx
Fx07 - LD Vx, DT
Fx0A - LD Vx, K
Fx15 - LD DT, Vx
Fx18 - LD ST, Vx
Fx1E - ADD I, Vx
Fx29 - LD F, Vx
Fx33 - LD B, Vx
Fx55 - LD [I], Vx
Fx65 - LD Vx, [I]
```