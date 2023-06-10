#include "common.h"

#include <float.h>

//--------------------------------------

void dead_blending_transition(
    float& ext_x, // Extrapolated position
    float& ext_v, // Extrapolated velocity 
    float& ext_t, // Time since transition
    float src_x,  // Current position
    float src_v)  // Current velocity
{
    ext_x = src_x;
    ext_v = src_v;
    ext_t = 0.0f;
}

static inline float smoothstep(float x)
{
    x = clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);  
}

void dead_blending_update(
    float& out_x,    // Output position
    float& out_v,    // Output velocity
    float& ext_x,    // Extrapolated position
    float& ext_v,    // Extrapolated velocity
    float& ext_t,    // Time since transition
    float in_x,      // Input position
    float in_v,      // Input velocity
    float blendtime, // Blend time
    float dt,        // Delta time
    float eps=1e-8f)
{    
    if (ext_t < blendtime)
    {
        ext_x += ext_v * dt;
        ext_t += dt;

        float alpha = smoothstep(ext_t / max(blendtime, eps));
        out_x = lerp(ext_x, in_x, alpha);
        out_v = lerp(ext_v, in_v, alpha);
    }
    else
    {
        out_x = in_x;
        out_v = in_v;
        ext_t = FLT_MAX;
    }
}

void dead_blending_update_decay(
    float& out_x,         // Output position
    float& out_v,         // Output velocity
    float& ext_x,         // Extrapolated position
    float& ext_v,         // Extrapolated velocity
    float& ext_t,         // Time since transition
    float in_x,           // Input position
    float in_v,           // Input velocity
    float blendtime,      // Blend time
    float decay_halflife, // Decay Halflife
    float dt,             // Delta time
    float eps=1e-8f)
{    
    if (ext_t < blendtime)
    {
        ext_v = damper_decay_exact(ext_v, decay_halflife, dt);
        ext_x += ext_v * dt;
        ext_t += dt;

        float alpha = smoothstep(ext_t / max(blendtime, eps));
        out_x = lerp(ext_x, in_x, alpha);
        out_v = lerp(ext_v, in_v, alpha);
    }
    else
    {
        out_x = in_x;
        out_v = in_v;
        ext_t = FLT_MAX;
    }
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

    float blendtime = 0.5f;
    float decay_halflife = 0.25f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float ext_x = 0.0;
    float ext_v = 0.0;
    float ext_t = FLT_MAX;
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
        
        //if (GuiButton((Rectangle){ 100, 75, 120, 20 }, "Transition"))
        if (GuiButton((Rectangle){ 100, 45, 120, 20 }, "Transition"))
        {
            inertialize_toggle = !inertialize_toggle;
            
            float src_x = x_prev[1];
            float src_v = (x_prev[1] - x_prev[2]) / (t_prev[1] - t_prev[2]);

            dead_blending_transition(
                ext_x, ext_v, ext_t,
                src_x, src_v);
        }
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "blendtime", TextFormat("%5.3f", blendtime), &blendtime, 0.0f, 1.0f);
        //GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);
        
        // Update Spring
        
        //SetTargetFPS(1.0f / dt);
        
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
        
        dead_blending_update(x, v, ext_x, ext_v, ext_t, g, gv, blendtime, dt);
        //dead_blending_update_decay(x, v, ext_x, ext_v, ext_t, g, gv, blendtime, decay_halflife, dt);
        
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