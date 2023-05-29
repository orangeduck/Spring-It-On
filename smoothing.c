#include "common.h"

enum
{
    HISTORY_MAX = 256
};

float x_prev[HISTORY_MAX];
float v_prev[HISTORY_MAX];
float t_prev[HISTORY_MAX];

int main(void)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - smoothing");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float v = 0.0;
    float g = x;
    float goalOffset = 600;

    float halflife = 0.1f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float noise = 0.0f;
    float jitter = 0.0f;
    
    SetTargetFPS(1.0f / dt);

    for (int i = 0; i < HISTORY_MAX; i++)
    {
        x_prev[i] = x;
        v_prev[i] = v;
        t_prev[i] = t;
    }
    
    while (!WindowShouldClose())
    {
        // Shift History
        
        for (int i = HISTORY_MAX - 1; i > 0; i--)
        {
            x_prev[i] = x_prev[i - 1];
            v_prev[i] = v_prev[i - 1];
            t_prev[i] = t_prev[i - 1];
        }
        
        // Get Goal
        
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            g = GetMousePosition().y;
        }
        
        g += noise * (((float)rand() / RAND_MAX) * 2.0f - 1.0);
        
        if (jitter)
        {
            g -= jitter;
            jitter = 0.0;
        }
        else if (rand() % (int)(0.5 / dt) == 0)
        {
            jitter = noise * 10.0f * (((float)rand() / RAND_MAX) * 2.0f - 1.0);
            g += jitter;
        }
        
        // Spring Damper
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 1.0f);
        GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);
        GuiSliderBar((Rectangle){ 100, 70, 120, 20 }, "noise", TextFormat("%5.3f", noise), &noise, 0.0f, 20.0f);
        
        // Update Spring
        
        SetTargetFPS(1.0f / dt);
        
        t += dt;
        
        simple_spring_damper_exact(x, v, g, halflife, dt);
        
        x_prev[0] = x;
        v_prev[0] = v;      
        t_prev[0] = t;
        
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawCircleV((Vector2){goalOffset, g}, 5, MAROON);
            DrawCircleV((Vector2){goalOffset, x}, 5, DARKBLUE);
            
            for (int i = 0; i < HISTORY_MAX - 1; i++)
            {
                Vector2 x_start = {goalOffset - (t - t_prev[i + 0]) * timescale, x_prev[i + 0]};
                Vector2 x_stop  = {goalOffset - (t - t_prev[i + 1]) * timescale, x_prev[i + 1]};
            
                DrawLineV(x_start, x_stop, BLUE);                
                DrawCircleV(x_start, 2, BLUE);
            }
            
        EndDrawing();
        
    }

    CloseWindow();

    return 0;
}