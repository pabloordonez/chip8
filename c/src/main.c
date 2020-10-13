#include "main.h"

int main()
{
    Cpu cpu;
    char **instructions = NULL;

    cpu_load_rom(&cpu, "../roms/IBM");

    u32 instruction_count = cpu_disassemble_code(&cpu, &instructions);

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1024;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("This is a raylib example", 10, 40, 20, DARKGRAY);

        for (u32 i = 0; i < instruction_count; i++)
        {
            if (60 + 20 * i > 720)
                break;

            if (instructions[i] != NULL)
            {
                DrawText(instructions[i], 10, 65 + 25 * i, 20, BLACK);
            }
        }

        DrawFPS(10, 10);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    cpu_free_disassembled_code(&instructions, instruction_count);

    return 0;
}