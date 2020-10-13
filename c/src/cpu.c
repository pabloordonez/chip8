#include "cpu.h"

#define PROGRAM_START 0x200

/**
 * The default font sprites.
 */
const u8 FONT_SET[80] = {
    0xf0,
    0x90,
    0x90,
    0x90,
    0xf0,
    0x20,
    0x60,
    0x20,
    0x20,
    0x70,
    0xf0,
    0x10,
    0xf0,
    0x80,
    0xf0,
    0xf0,
    0x10,
    0xf0,
    0x10,
    0xf0,
    0x90,
    0x90,
    0xf0,
    0x10,
    0x10,
    0xf0,
    0x80,
    0xf0,
    0x10,
    0xf0,
    0xf0,
    0x80,
    0xf0,
    0x90,
    0xf0,
    0xf0,
    0x10,
    0x20,
    0x40,
    0x40,
    0xf0,
    0x90,
    0xf0,
    0x90,
    0xf0,
    0xf0,
    0x90,
    0xf0,
    0x10,
    0xf0,
    0xf0,
    0x90,
    0xf0,
    0x90,
    0x90,
    0xe0,
    0x90,
    0xe0,
    0x90,
    0xe0,
    0xf0,
    0x80,
    0x80,
    0x80,
    0xf0,
    0xe0,
    0x90,
    0x90,
    0x90,
    0xe0,
    0xf0,
    0x80,
    0xf0,
    0x80,
    0xf0,
    0xf0,
    0x80,
    0xf0,
    0x80,
    0x80,
};

static inline int overflow_add(u8 *result, u8 a, u8 b);
static inline void cpu_move_program_counter_forward(Cpu *cpu);
static inline void cpu_move_program_counter_backward(Cpu *cpu);
static inline void op_sys_nnn(Cpu *cpu, u16 nnn);
static inline void op_cls(Cpu *cpu);
static inline void op_ret(Cpu *cpu);
static inline void op_jp_nnn(Cpu *cpu, u16 nnn);
static inline void op_jp_v0_nnn(Cpu *cpu, u16 nnn);
static inline void op_call_nnn(Cpu *cpu, u16 nnn);
static inline void op_se_vx_kk(Cpu *cpu, u8 x, u8 kk);
static inline void op_se_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_sne_vx_kk(Cpu *cpu, u8 x, u8 kk);
static inline void op_sne_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_skp_vx(Cpu *cpu, u8 x);
static inline void op_skpn_vx(Cpu *cpu, u8 x);
static inline void op_ld_i_nnn(Cpu *cpu, u16 nnn);
static inline void op_ld_vx_kk(Cpu *cpu, u8 x, u8 kk);
static inline void op_ld_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_ld_vx_dt(Cpu *cpu, u8 x);
static inline void op_ld_dt_vx(Cpu *cpu, u8 x);
static inline void op_ld_st_vx(Cpu *cpu, u8 x);
static inline void op_ld_f_vx(Cpu *cpu, u8 x);
static inline void op_ld_b_vx(Cpu *cpu, u8 x);
static inline void op_ld_i_vx(Cpu *cpu, u8 x);
static inline void op_ld_vx_i(Cpu *cpu, u8 x);
static inline void op_ld_vx_key(Cpu *cpu, u8 x);
static inline void op_add_vx_kk(Cpu *cpu, u8 x, u8 kk);
static inline void op_add_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_add_i_vx(Cpu *cpu, u8 x);
static inline void op_or_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_and_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_xor_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_sub_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_subn_vx_vy(Cpu *cpu, u8 x, u8 y);
static inline void op_shr_vx(Cpu *cpu, u8 x);
static inline void op_shl_vx(Cpu *cpu, u8 x);
static inline void op_rnd_vx_kk(Cpu *cpu, u8 x, u8 kk);
static inline void op_drw_vx_vy_n(Cpu *cpu, u8 x, u8 y, u8 n);

void cpu_reset(Cpu *cpu)
{
    // initializes the memory.
    memset(cpu->memory, 0, sizeof(cpu->memory));
    memset(cpu->stack, 0, sizeof(cpu->stack));

    // sets the font sprites
    memcpy(cpu->memory, FONT_SET, sizeof(FONT_SET));

    cpu->program_counter = PROGRAM_START;
    cpu->stack_pointer = 0;
    cpu->delay_timer = 0;
    cpu->sound_timer = 0;
    cpu->index_register = 0;

    gpu_reset(&cpu->gpu);
    keyboard_reset(&cpu->keyboard);
}

void cpu_load_rom(Cpu *cpu, const char *file_name)
{
    cpu_reset(cpu);

    // moves rom from file to ram
    FILE *file = fopen(file_name, "rb");
    u16 i = PROGRAM_START;

    if (file == NULL)
    {
        perror("Unable to load the rom");
        return;
    }

    while (!feof(file))
    {
        cpu->memory[i++] = fgetc(file);
    }

    fclose(file);

    char instruction[100];
    i = PROGRAM_START;
    file = fopen("disassemble.txt", "wt");

    if (file == NULL)
    {
        perror("Unable to create the disassemble file");
        return;
    }

    // disassemble the op_code
    while (i < sizeof(cpu->memory) / 2)
    {
        memset(instruction, 0, sizeof(instruction));
        u16 op_code = (cpu->memory[i] << 8) | cpu->memory[i + 1];

        if (op_code != 0)
        {
            cpu_disassemble_op(cpu, op_code, instruction);
            fprintf(file, "%X [%X]: %s\n", i, op_code, instruction);
        }

        i += 2;
    }

    fclose(file);
}

void cpu_execute_op(Cpu *cpu, const u16 op_code)
{
    u8 op1 = ((op_code & 0xF000) >> 12);
    u8 op2 = ((op_code & 0x0F00) >> 8);
    u8 op3 = ((op_code & 0x00F0) >> 4);
    u8 op4 = (op_code & 0x000F);
    u16 nnn = op_code & 0x0FFF;
    u8 kk = (op_code & 0x00FF);

    if (op_code == 0x00E0)
        op_cls(cpu);

    else if (op_code == 0x00EE)
        op_ret(cpu);

    else if (op1 == 0x00)
        op_sys_nnn(cpu, nnn);

    else if (op1 == 0x01)
        op_jp_nnn(cpu, nnn);

    else if (op1 == 0x02)
        op_call_nnn(cpu, nnn);

    else if (op1 == 0x03)
        op_se_vx_kk(cpu, op2, kk);

    else if (op1 == 0x04)
        op_sne_vx_kk(cpu, op2, kk);

    else if (op1 == 0x05 && op4 == 0x00)
        op_se_vx_vy(cpu, op2, op3);

    else if (op1 == 0x06)
        op_ld_vx_kk(cpu, op2, kk);

    else if (op1 == 0x07)
        op_add_vx_kk(cpu, op2, kk);

    else if (op1 == 0x08 && op4 == 0x00)
        op_ld_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x01)
        op_or_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x02)
        op_and_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x03)
        op_xor_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x04)
        op_add_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x05)
        op_sub_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x06)
        op_shr_vx(cpu, op2);

    else if (op1 == 0x08 && op4 == 0x07)
        op_subn_vx_vy(cpu, op2, op3);

    else if (op1 == 0x08 && op4 == 0x0E)
        op_shl_vx(cpu, op2);

    else if (op1 == 0x09 && op4 == 0x00)
        op_sne_vx_vy(cpu, op2, op3);

    else if (op1 == 0x0A)
        op_ld_i_nnn(cpu, nnn);

    else if (op1 == 0x0B)
        op_jp_v0_nnn(cpu, nnn);

    else if (op1 == 0x0C)
        op_rnd_vx_kk(cpu, op2, kk);

    else if (op1 == 0x0D)
        op_drw_vx_vy_n(cpu, op2, op3, op4);

    else if (op1 == 0x0E && op3 == 0x09 && op4 == 0x0E)
        op_skp_vx(cpu, op2);

    else if (op1 == 0x0E && op3 == 0x0A && op4 == 0x01)
        op_skpn_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x00 && op4 == 0x07)
        op_ld_vx_dt(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x00 && op4 == 0x0A)
        op_ld_vx_key(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x01 && op4 == 0x05)
        op_ld_dt_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x01 && op4 == 0x08)
        op_ld_st_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x01 && op4 == 0x0E)
        op_add_i_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x02 && op4 == 0x09)
        op_ld_f_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x03 && op4 == 0x03)
        op_ld_b_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x05 && op4 == 0x05)
        op_ld_i_vx(cpu, op2);

    else if (op1 == 0x0F && op3 == 0x06 && op4 == 0x05)
        op_ld_vx_i(cpu, op2);
}

void cpu_disassemble_op(Cpu *cpu, const u16 op_code, char *instruction)
{
    u8 op1 = ((op_code & 0xF000) >> 12);
    u8 op2 = ((op_code & 0x0F00) >> 8);
    u8 op3 = ((op_code & 0x00F0) >> 4);
    u8 op4 = (op_code & 0x000F);
    u16 nnn = op_code & 0x0FFF;
    u8 kk = (op_code & 0x00FF);

    if (op_code == 0x00E0)
        sprintf(instruction, "CLS");

    else if (op_code == 0x00EE)
        sprintf(instruction, "RET");

    else if (op1 == 0x00)
        sprintf(instruction, "SYS  %X", nnn);

    else if (op1 == 0x01)
        sprintf(instruction, "JMP  %X", nnn);

    else if (op1 == 0x02)
        sprintf(instruction, "CALL %X", nnn);

    else if (op1 == 0x03)
        sprintf(instruction, "SE   V%X, %X", op2, kk);

    else if (op1 == 0x04)
        sprintf(instruction, "SNE  V%X, %X", op2, kk);

    else if (op1 == 0x05 && op4 == 0x00)
        sprintf(instruction, "SE   V%X, V%X", op2, op3);

    else if (op1 == 0x06)
        sprintf(instruction, "LD   V%X, %X", op2, kk);

    else if (op1 == 0x07)
        sprintf(instruction, "ADD  V%X, %X", op2, kk);

    else if (op1 == 0x08 && op4 == 0x00)
        sprintf(instruction, "LD   V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x01)
        sprintf(instruction, "OR   V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x02)
        sprintf(instruction, "AND  V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x03)
        sprintf(instruction, "XOR  V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x04)
        sprintf(instruction, "ADD  V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x05)
        sprintf(instruction, "SUB  V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x06)
        sprintf(instruction, "SHR  V%X", op2);

    else if (op1 == 0x08 && op4 == 0x07)
        sprintf(instruction, "SUBN V%X, V%X", op2, op3);

    else if (op1 == 0x08 && op4 == 0x0E)
        sprintf(instruction, "SHL  V%X", op2);

    else if (op1 == 0x09 && op4 == 0x00)
        sprintf(instruction, "SNE  V%X, V%X", op2, op3);

    else if (op1 == 0x0A)
        sprintf(instruction, "LD   I, %X", nnn);

    else if (op1 == 0x0B)
        sprintf(instruction, "JMP  V0, %X", nnn);

    else if (op1 == 0x0C)
        sprintf(instruction, "RND  V%X, %X", op2, kk);

    else if (op1 == 0x0D)
        sprintf(instruction, "DRW  V%X, V%X, %X", op2, op3, op4);

    else if (op1 == 0x0E && op3 == 0x09 && op4 == 0x0E)
        sprintf(instruction, "SKP  V%X", op2);

    else if (op1 == 0x0E && op3 == 0x0A && op4 == 0x01)
        sprintf(instruction, "SKPN V%X", op2);

    else if (op1 == 0x0F && op3 == 0x00 && op4 == 0x07)
        sprintf(instruction, "LD   V%X, DT", op2);

    else if (op1 == 0x0F && op3 == 0x00 && op4 == 0x0A)
        sprintf(instruction, "LD   V%X, KEY", op2);

    else if (op1 == 0x0F && op3 == 0x01 && op4 == 0x05)
        sprintf(instruction, "LD   DT, V%X", op2);

    else if (op1 == 0x0F && op3 == 0x01 && op4 == 0x08)
        sprintf(instruction, "LD   ST, V%X", op2);

    else if (op1 == 0x0F && op3 == 0x01 && op4 == 0x0E)
        sprintf(instruction, "ADD  I, V%X", op2);

    else if (op1 == 0x0F && op3 == 0x02 && op4 == 0x09)
        sprintf(instruction, "LD   F, V%X", op2);

    else if (op1 == 0x0F && op3 == 0x03 && op4 == 0x03)
        sprintf(instruction, "LD   B, V%X", op2);

    else if (op1 == 0x0F && op3 == 0x05 && op4 == 0x05)
        sprintf(instruction, "LD   [I], V%X", op2);

    else if (op1 == 0x0F && op3 == 0x06 && op4 == 0x05)
        sprintf(instruction, "LD   V%X, [I]", op2);

    else
        sprintf(instruction, "-- Unknown --");
}

u32 cpu_disassemble_code(Cpu *cpu, char ***instructions)
{
    u32 i = 0;
    u32 j = 0;
    u32 size = sizeof(cpu->memory);
    u32 instruction_count = (size - PROGRAM_START) / 2;
    u32 instruction_size = instruction_count * sizeof(char *);
    (*instructions) = malloc(instruction_size);

    // disassemble the op_code
    while (j < instruction_count)
    {
        u16 op_code = (cpu->memory[i + PROGRAM_START] << 8) | cpu->memory[i + PROGRAM_START + 1];

        if (op_code != 0)
        {
            (*instructions)[j] = malloc(100);
            memset((*instructions)[j], 0, 100);
            cpu_disassemble_op(cpu, op_code, (*instructions)[j]);
        }
        else
        {
            (*instructions)[j] = NULL;
        }

        i += 2;
        j += 1;
    }

    return instruction_count;
}

void cpu_free_disassembled_code(char*** instructions, u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        if ((*instructions)[i] != NULL)
        {
            free((*instructions)[i]);
        }
    }

    free((*instructions));
}

static inline int overflow_add(u8 *result, u8 a, u8 b)
{
    *result = a + b;

    if (a > 0 && b > 0 && *result < 0)
        return 1;

    if (a < 0 && b < 0 && *result > 0)
        return 1;

    return 0;
}

static inline void cpu_move_program_counter_forward(Cpu *cpu)
{
    // we move two bytes ahead: each instruction is 2 bytes,
    // so we are moving one instruction ahead.
    cpu->program_counter += 2;
}

static inline void cpu_move_program_counter_backward(Cpu *cpu)
{
    // we move two bytes ahead: each instruction is 2 bytes,
    // so we are moving one instruction ahead.
    cpu->program_counter -= 2;
}

static inline void op_sys_nnn(Cpu *cpu, u16 nnn)
{
    cpu->program_counter = nnn;
}

static inline void op_cls(Cpu *cpu)
{
    gpu_clear_memory(&cpu->gpu);
}

static inline void op_ret(Cpu *cpu)
{
    cpu->program_counter = cpu->stack[cpu->stack_pointer];
    cpu->stack_pointer -= 1;
}

static inline void op_jp_nnn(Cpu *cpu, u16 nnn)
{
    cpu->program_counter = nnn;
}

static inline void op_jp_v0_nnn(Cpu *cpu, u16 nnn)
{
    cpu->program_counter = nnn + (u16)cpu->value_registers[0];
}

static inline void op_call_nnn(Cpu *cpu, u16 nnn)
{
    cpu->stack_pointer += 1;
    cpu->stack[cpu->stack_pointer] = cpu->program_counter;
    cpu->program_counter = nnn;
}

static inline void op_se_vx_kk(Cpu *cpu, u8 x, u8 kk)
{
    if (cpu->value_registers[x] == kk)
    {
        cpu_move_program_counter_forward(cpu);
    }
}

static inline void op_se_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    if (cpu->value_registers[x] == cpu->value_registers[y])
    {
        cpu_move_program_counter_forward(cpu);
    }
}

static inline void op_sne_vx_kk(Cpu *cpu, u8 x, u8 kk)
{
    if (cpu->value_registers[x] != kk)
    {
        cpu_move_program_counter_forward(cpu);
    }
}

static inline void op_sne_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    if (cpu->value_registers[x] != cpu->value_registers[y])
    {
        cpu_move_program_counter_forward(cpu);
    }
}

static inline void op_skp_vx(Cpu *cpu, u8 x)
{
    if (keyboard_is_key_pressed(&cpu->keyboard, cpu->value_registers[x]))
    {
        cpu_move_program_counter_forward(cpu);
    }
}

static inline void op_skpn_vx(Cpu *cpu, u8 x)
{
    if (!keyboard_is_key_pressed(&cpu->keyboard, cpu->value_registers[x]))
    {
        cpu_move_program_counter_forward(cpu);
    }
}

static inline void op_ld_i_nnn(Cpu *cpu, u16 nnn)
{
    cpu->index_register = nnn;
}

static inline void op_ld_vx_kk(Cpu *cpu, u8 x, u8 kk)
{
    cpu->value_registers[x] = kk;
}

static inline void op_ld_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    cpu->value_registers[x] = cpu->value_registers[y];
}

static inline void op_ld_vx_dt(Cpu *cpu, u8 x)
{
    cpu->value_registers[x] = cpu->delay_timer;
}

static inline void op_ld_dt_vx(Cpu *cpu, u8 x)
{
    cpu->delay_timer = cpu->value_registers[x];
}

static inline void op_ld_st_vx(Cpu *cpu, u8 x)
{
    cpu->sound_timer = cpu->value_registers[x];
}

static inline void op_ld_f_vx(Cpu *cpu, u8 x)
{
    cpu->index_register = ((u16)cpu->value_registers[x]) * 5;
}

static inline void op_ld_b_vx(Cpu *cpu, u8 x)
{
    u16 i = cpu->index_register;
    u8 a = x / 100;
    x = x - a * 100;
    u8 b = x / 10;
    x = x - b * 10;

    cpu->memory[i + 0] = a;
    cpu->memory[i + 1] = b;
    cpu->memory[i + 2] = x;
}

static inline void op_ld_i_vx(Cpu *cpu, u8 x)
{
    u16 i = cpu->index_register;
    memcpy(&cpu->memory[i], &cpu->value_registers[0], x);
}

static inline void op_ld_vx_i(Cpu *cpu, u8 x)
{
    u16 i = cpu->index_register;
    memcpy(&cpu->value_registers[0], &cpu->memory[i], x);
}

void op_ld_vx_key(Cpu *cpu, u8 x)
{
    if (!keyboard_is_any_key_pressed(&cpu->keyboard))
    {
        cpu_move_program_counter_backward(cpu);
        return;
    }

    i8 key_index = keyboard_get_key_pressed_index(&cpu->keyboard);

    if (key_index < 0)
        return;

    cpu->value_registers[x] = key_index;
}

static inline void op_add_vx_kk(Cpu *cpu, u8 x, u8 kk)
{
    cpu->value_registers[x] += kk;
}

static inline void op_add_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    u8 result = 0;
    cpu->value_registers[0x0F] = overflow_add(&result, cpu->value_registers[x], cpu->value_registers[y]);
    cpu->value_registers[x] = result;
}

static inline void op_add_i_vx(Cpu *cpu, u8 x)
{
    cpu->index_register += cpu->value_registers[x];
}

static inline void op_or_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    cpu->value_registers[x] |= cpu->value_registers[y];
}

static inline void op_and_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    cpu->value_registers[x] &= cpu->value_registers[y];
}

static inline void op_xor_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    cpu->value_registers[x] ^= cpu->value_registers[y];
}

static inline void op_sub_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    u8 vx = cpu->value_registers[x];
    u8 vy = cpu->value_registers[y];

    cpu->value_registers[0x0F] = vx > vy;
    cpu->value_registers[x] = vx - vy;
}

static inline void op_subn_vx_vy(Cpu *cpu, u8 x, u8 y)
{
    u8 vx = cpu->value_registers[x];
    u8 vy = cpu->value_registers[y];

    cpu->value_registers[0x0F] = vy > vx;
    cpu->value_registers[x] = vy - vx;
}

static inline void op_shr_vx(Cpu *cpu, u8 x)
{
    cpu->value_registers[0x0F] = cpu->value_registers[x] & 0x00;
    cpu->value_registers[x] >>= 1;
}

static inline void op_shl_vx(Cpu *cpu, u8 x)
{
    cpu->value_registers[0x0F] = cpu->value_registers[x] & 0x80;
    cpu->value_registers[x] >>= 1;
}

static inline void op_rnd_vx_kk(Cpu *cpu, u8 x, u8 kk)
{
    cpu->value_registers[x] = (rand() % 255) & kk;
}

static inline void op_drw_vx_vy_n(Cpu *cpu, u8 x, u8 y, u8 n)
{
    u16 i = cpu->index_register;
    cpu->value_registers[0x0F] = gpu_draw_sprite(&cpu->gpu, x, y, cpu->memory, i, n);
}
