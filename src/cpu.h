#ifndef __CPU_H__
#define __CPU_H__

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "types.h"
#include "keyboard.h"
#include "gpu.h"

/**
 * Defines a cpu device.
 * The main processing unit.
 */
typedef struct Cpu
{
    u8 memory[4096];
    u8 value_registers[16];
    u16 stack[16];
    u16 program_counter;
    u8 stack_pointer;
    u16 index_register;
    u8 sound_timer;
    u8 delay_timer;
    Gpu gpu;
    Keyboard keyboard;
} Cpu;

void cpu_reset(Cpu *cpu);

void cpu_load_rom(Cpu* cpu, const char* file_name);

void cpu_execute_op(Cpu* cpu, const u16 op_code);

void cpu_clock(Cpu* cpu);

void cpu_disassemble_op(const Cpu* cpu, const u16 op_code, char* instruction);

u32 cpu_disassemble_code(const Cpu* cpu, char*** instructions);

void cpu_free_disassembled_code(char*** instructions, u32 count);

u16 cpu_get_instruction_pointer_index(const Cpu* cpu);

#endif /*__CPU_H__*/