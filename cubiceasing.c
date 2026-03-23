#include "common.h"

enum
{
    HISTORY_MAX = 256
};

float x_prev[HISTORY_MAX];
float t_prev[HISTORY_MAX];

float cubic(float t, float v, float g)
{
    if (t > 1.0f)
    {
        return g;
    }
    else
    {
        float w1 = 3*t*t - 2*t*t*t;
        float w2 = t*t*t - 2*t*t + t;
        return w1*g + w2*v;
    }
}

float cubic_dt(float t, float v, float g)
{
    if (t > 1.0f)
    {
        return 0.0f;
    }
    else
    {
        float q1 = 6*t - 6*t*t;
        float q2 = 3*t*t - 4*t + 1;
        return q1*g + q2*v;
    }
}

int main(void)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - cubic easing");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float g = x;
    float goalOffset = 400;

    float blendtime = 1.0f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float ct = 0.0f;
    float cv = 0.0f;
    float co = x;

    bool draw_fit = true;

    SetTargetFPS(1.0f / dt);

    for (int i = 0; i < HISTORY_MAX; i++)
    {
        x_prev[i] = x;
        t_prev[i] = t;
    }    

    while (!WindowShouldClose())
    {
        // Shift History
        
        for (int i = HISTORY_MAX - 1; i > 0; i--)
        {
            x_prev[i] = x_prev[i - 1];
            t_prev[i] = t_prev[i - 1];
        }

        // UI
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "blendtime", TextFormat("%5.3f", blendtime), &blendtime, 0.0f, 2.0f);
        GuiDrawText(TextFormat("t=% 5.2f", ct), (Rectangle){ 275, 20, 120, 20 }, TEXT_ALIGN_LEFT, DARKGRAY);
        GuiCheckBox((Rectangle){ 400, 20, 20, 20 }, "draw fit", &draw_fit);

        // Check if target changed
        
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            if (g != GetMousePosition().y)
            {
                // Get current location and velocity
                float nx = co + cubic(ct, cv, g - co);
                float nv = cubic_dt(ct, cv, g - co);
                
                // Reset to the new target, offset, velocity, and time
                g = GetMousePosition().y;
                co = nx;
                cv = nv;
                ct = 0.0;
            }
        }

        // Update the cubic time
        ct += dt / max(blendtime, 1e-4f);
        
        // Update the cubic value
        x = co + cubic(ct, cv, g - co);
        
        // Update Time
        t += dt;
        
        x_prev[0] = x;
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

            if (draw_fit)
            {
                for (int i = 0; i < 100; i++)
                {
                    float pt0 = ((i + 0) / 100.0f) * 2.0f - 1.0f;
                    float pt1 = ((i + 1) / 100.0f) * 2.0f - 1.0f;
                    
                    float py0 = co + cubic(ct + pt0, cv, g - co);
                    float py1 = co + cubic(ct + pt1, cv, g - co);
                    
                    Vector2 p_start = { goalOffset + pt0 * timescale, py0 };
                    Vector2 p_stop = { goalOffset + pt1 * timescale, py1 };
                    
                    DrawLineV(p_start, p_stop, PURPLE);
                }     
            }
            
        EndDrawing();
        
    }

    CloseWindow();

    return 0;
}