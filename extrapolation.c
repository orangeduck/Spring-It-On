#include "common.h"

#include <float.h>

//--------------------------------------

void extrapolate(
    float& x,
    float& v,
    float dt,
    float halflife,
    float eps = 1e-5f)
{
    float y = 0.69314718056f / (halflife + eps);
    x = x + (v / (y + eps)) * (1.0f - fast_negexp(y * dt));
    v = v * fast_negexp(y * dt);
}

void extrapolate_function(float& g, float& gv, float t, float freq, float amp, float phase, float off)
{
    g = amp * sin(t * freq + phase) + off;
    gv = amp * freq * cos(t * freq + phase);
}

void extrapolate_function1(float& g, float& gv, float t)
{
    float g0, gv0, g1, gv1, g2, gv2;
  
    extrapolate_function(g0, gv0, t, 2.0f * M_PI * 1.5, 40.0, 23.213123, 0);
    extrapolate_function(g1, gv1, t, 2.0f * M_PI * 3.4, 14.0, 912.2381, 0);
    extrapolate_function(g2, gv2, t, 2.0f * M_PI * 0.4, 21.0, 452.2381, 0);
    
    g = 200 + g0 + g1 + g2;
    gv = gv0 + gv1 + gv2;
}

//--------------------------------------

enum
{
    HISTORY_MAX = 256
};

float x_prev[HISTORY_MAX];
float v_prev[HISTORY_MAX];
float t_prev[HISTORY_MAX];
float g_prev[HISTORY_MAX];

int main(void)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - extrapolation");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float v = 0.0;
    float g = x;
    float goalOffset = 600;

    float halflife = 0.2f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    bool extrapolation_toggle = false;

    SetTargetFPS(1.0f / dt);

    for (int i = 0; i < HISTORY_MAX; i++)
    {
        x_prev[i] = x;
        v_prev[i] = v;
        t_prev[i] = t;
        g_prev[i] = x;
    }
    
    while (!WindowShouldClose())
    {
        // Shift History
        
        for (int i = HISTORY_MAX - 1; i > 0; i--)
        {
            x_prev[i] = x_prev[i - 1];
            v_prev[i] = v_prev[i - 1];
            t_prev[i] = t_prev[i - 1];
            g_prev[i] = g_prev[i - 1];
        }
        
        //if (GuiButton((Rectangle){ 100, 75, 120, 20 }, "Extrapolate"))
        if (GuiButton((Rectangle){ 100, 45, 120, 20 }, "Extrapolate"))
        {
            extrapolation_toggle = !extrapolation_toggle;
        }
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 0.5f);
        //GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);
        
        // Update Spring
        
        //SetTargetFPS(1.0f / dt);
        
        t += dt;
        
        float gv = 0.0f;

        extrapolate_function1(g, gv, t);
        
        if (extrapolation_toggle)
        {
            extrapolate(x, v, dt, halflife);
        }
        else
        {
            x = g;
            v = gv;
        }
        
        x_prev[0] = x;
        v_prev[0] = v;      
        t_prev[0] = t;
        g_prev[0] = g;
        
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
            
            for (int i = 0; i < HISTORY_MAX - 1; i++)
            {
                Vector2 g_start = {goalOffset - (t - t_prev[i + 0]) * timescale, g_prev[i + 0]};
                Vector2 g_stop  = {goalOffset - (t - t_prev[i + 1]) * timescale, g_prev[i + 1]};
                
                DrawLineV(g_start, g_stop, MAROON);                
                DrawCircleV(g_start, 2, MAROON);
            }
            
        EndDrawing();
        
    }

    CloseWindow();

    return 0;
}