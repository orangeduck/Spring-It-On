#include "common.h"

//--------------------------------------

void inertialize_transition(
    float& off_x, float& off_v, 
    float src_x, float src_v,
    float dst_x, float dst_v)
{
    off_x = (src_x + off_x) - dst_x;
    off_v = (src_v + off_v) - dst_v;
}

void inertialize_update(
    float& out_x, float& out_v,
    float& off_x, float& off_v,
    float in_x, float in_v,
    float halflife,
    float dt)
{
    decay_spring_damper_exact(off_x, off_v, halflife, dt);
    out_x = in_x + off_x;
    out_v = in_v + off_v;
}

void inertialize_function(float& g, float& gv, float t, float freq, float amp, float phase, float off)
{
    g = amp * sin(t * freq + phase) + off;
    gv = amp * freq * cos(t * freq + phase);
}

void inertialize_function1(float& g, float& gv, float t)
{
    inertialize_function(g, gv, t, 2.0f * M_PI * 1.25, 74.0, 23.213123, 254);
}

void inertialize_function2(float& g, float& gv, float t)
{
    inertialize_function(g, gv, t, 2.0f * M_PI * 3.4, 28.0, 912.2381, 113);
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

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - inertialization");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float v = 0.0;
    float g = x;
    float goalOffset = 600;

    float halflife = 0.1f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float off_x = 0.0;
    float off_v = 0.0;
    bool inertialize_toggle = false;

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
        
        if (GuiButton((Rectangle){ 100, 75, 120, 20 }, "Transition"))
        {
            inertialize_toggle = !inertialize_toggle;
            
            float src_x = g_prev[1];
            float src_v = (g_prev[1] - g_prev[2]) / (t_prev[1] - t_prev[2]);
            float dst_x, dst_v;
            
            if (inertialize_toggle)
            {
                inertialize_function1(dst_x, dst_v, t);                    
            }
            else
            {
                inertialize_function2(dst_x, dst_v, t);
            }
        
            inertialize_transition(
                off_x, off_v,
                src_x, src_v,
                dst_x, dst_v);
        }
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 1.0f);
        GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);
        
        // Update Spring
        
        SetTargetFPS(1.0f / dt);
        
        t += dt;
        
        float gv = 0.0f;
        if (inertialize_toggle)
        {
            inertialize_function1(g, gv, t);
        }
        else
        {
            inertialize_function2(g, gv, t);
        }
        
        inertialize_update(x, v, off_x, off_v, g, gv, halflife, dt);
        
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