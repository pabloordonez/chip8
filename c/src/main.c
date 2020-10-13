#include "main.h"

#define WIDTH 1024
#define HEIGHT 720
#define FPS 60

void draw_instructions(char **instructions, const u32 instruction_count, const Cpu *cpu)
{
    const u32 width = 240;
    const i32 sx = WIDTH - width;
    const i32 sy = 10;
    const i32 font_size = 20;
    const u16 current_instruction_index = cpu_get_instruction_pointer_index(cpu);
    const u32 from = current_instruction_index < 5 ? 0 : current_instruction_index - 5;
    i32 y = sy + 10;

    DrawRectangleLines(sx - 10, sy, width, HEIGHT - 20, (Color){0, 0, 0, 50});
    DrawText("INSTRUCTIONS", sx, y, font_size, BLACK);
    y += font_size + 10;

    printf("from %d", from);

    for (u32 i = 0; i < 26 && i + from < instruction_count; i++)
    {
        if (current_instruction_index == i + from)
        {
            DrawTriangle((Vector2){sx - 5, y + 10}, (Vector2){sx - 25, y}, (Vector2){sx - 25, y + 20}, BLUE);
            DrawRectangle(sx, y, width - 10, font_size, (Color){0, 121, 241, 50});
        }

        DrawText((instructions[i + from] != NULL) ? instructions[i + from] : "NONE", sx, y, font_size, GRAY);
        y += font_size + 5;
    }
}

void draw_cpu_state(Cpu *cpu)
{
    const i32 width = 750;
    const i32 height = 240;
    const i32 sx = 20;
    const i32 sy = HEIGHT - height;
    const i32 font_size = 20;
    char buffer[32];
    i32 y = 0;
    i32 x = 0;

    DrawRectangleLines(sx - 10, sy - 10, width, height, (Color){0, 0, 0, 50});
    DrawText("CPU", sx, sy, font_size, BLACK);
    y += 25;

    sprintf(buffer, "I: %X", cpu->index_register);
    DrawText(buffer, sx, sy + y, font_size, GRAY);
    y += 25;

    const u16 current_instruction_index = cpu_get_instruction_pointer_index(cpu);
    sprintf(buffer, "PC: %X", /*cpu->program_counter*/ current_instruction_index);
    DrawText(buffer, sx, sy + y, font_size, GRAY);
    y += 25;

    sprintf(buffer, "SP: %X", cpu->stack_pointer);
    DrawText(buffer, sx, sy + y, font_size, GRAY);
    y += 25;

    sprintf(buffer, "ST: %X", cpu->sound_timer);
    DrawText(buffer, sx, sy + y, font_size, GRAY);
    y += 25;

    sprintf(buffer, "DT: %X", cpu->delay_timer);
    DrawText(buffer, sx, sy + y, font_size, GRAY);
    y += 25;

    y = 25;
    x = 150;

    for (u8 i = 0; i < 0x10; i++)
    {
        sprintf(buffer, "V%X: %X", i, cpu->value_registers[i]);
        DrawText(buffer, sx + x, sy + y, font_size, GRAY);
        y += 25;

        if (i == 7)
        {
            x += 150;
            y = 25;
        }
    }

    y = 25;
    x = 450;

    for (u8 i = 0; i < 0x10; i++)
    {
        if (i == cpu->stack_pointer)
        {
            DrawTriangle((Vector2){sx + x - 5, sy + y + 10}, (Vector2){sx + x - 25, sy + y}, (Vector2){sx + x - 25, sy + y + 20}, BLUE);
            DrawRectangle(sx + x, sy + y, 140, font_size, (Color){0, 121, 241, 50});
        }

        sprintf(buffer, "S%X: %X", i, cpu->stack[i]);
        DrawText(buffer, sx + x, sy + y, font_size, GRAY);

        y += 25;

        if (i == 7)
        {
            x += 150;
            y = 25;
        }
    }
}

int main()
{
    Cpu cpu;
    char **instructions = NULL;
    bool running = false;

    cpu_load_rom(&cpu, "../roms/CONNECT4");
    u32 instruction_count = cpu_disassemble_code(&cpu, &instructions);

    InitWindow(WIDTH, HEIGHT, "Chip 8");
    SetTargetFPS(FPS);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F10) && !running)
        {
            cpu_clock(&cpu);
        }

        if (IsKeyPressed(KEY_F5))
        {
            running = !running;
        }

        if (running)
        {
            cpu_clock(&cpu);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_cpu_state(&cpu);
        draw_instructions(instructions, instruction_count, &cpu);

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    cpu_free_disassembled_code(&instructions, instruction_count);

    return 0;
}