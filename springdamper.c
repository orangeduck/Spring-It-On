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

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - springdamper");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float v = 0.0;
    float g = x;
    float goalOffset = 600;

    //float stiffness = 15.0f;
    //float damping = 2.0f;
    //float frequency = 2.0f;
    float damping_ratio = 1.0f;
    float halflife = 0.1f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
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
        
        // Spring Damper
        
        //GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "stiffness", TextFormat("%5.3f", stiffness), &stiffness, 0.01f, 30.0f);
        //GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "frequency", TextFormat("%5.3f", frequency), &frequency, 0.0f, 3.0f);
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "damping ratio", TextFormat("%5.3f", damping_ratio), &damping_ratio, 0.0f, 2.0f);
        //GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "damping", TextFormat("%5.3f", damping), &damping, 0.01f, 30.0f);
        GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 1.0f);
        GuiSliderBar((Rectangle){ 100, 70, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);
        
        // Update Spring
        
        SetTargetFPS(1.0f / dt);
        
        t += dt;
        //spring_damper_bad(x, v, g, 0.0f, stiffness, damping, dt);
        //spring_damper_exact(x, v, g, 0.0f, stiffness, damping, dt);
        //spring_damper_exact(x, v, g, 0.0f, frequency, halflife, dt);
        spring_damper_exact_ratio(x, v, g, 0.0f, damping_ratio, halflife, dt);
        //critical_spring_damper_exact(x, v, g, 0.0f, halflife, dt);
        //simple_spring_damper_exact(x, v, g, halflife, dt);
        //decay_spring_damper_exact(x, v, halflife, dt);
        
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