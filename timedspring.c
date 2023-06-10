#include "common.h"

//--------------------------------------

void timed_spring_damper_exact(
    float& x,
    float& v,
    float& xi,
    float x_goal,
    float t_goal,
    float halflife,
    float dt,
    float apprehension = 2.0f)
{
    float min_time = t_goal > dt ? t_goal : dt;
    
    float v_goal = (x_goal - xi) / min_time;
    
    float t_goal_future = dt + apprehension * halflife;
    float x_goal_future = t_goal_future < t_goal ?
        xi + v_goal * t_goal_future : x_goal;
        
    simple_spring_damper_exact(x, v, x_goal_future, halflife, dt);
    
    xi += v_goal * dt;
}

//--------------------------------------

enum
{
    HISTORY_MAX = 256
};

float x_prev[HISTORY_MAX];
float v_prev[HISTORY_MAX];
float t_prev[HISTORY_MAX];

float xi_prev[HISTORY_MAX];

int main(void)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - timedspring");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float v = 0.0;
    float g = x;
    float goalOffset = 600;

    float halflife = 0.1f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;

    float goal_time = 1.0f;
    float ti = 0.0;
    float apprehension = 2.0f;

    float xi = x;

    SetTargetFPS(1.0f / dt);

    for (int i = 0; i < HISTORY_MAX; i++)
    {
        x_prev[i] = x;
        v_prev[i] = v;
        t_prev[i] = t;
        
        xi_prev[i] = x;
    }

    while (!WindowShouldClose())
    {
        // Shift History
        
        for (int i = HISTORY_MAX - 1; i > 0; i--)
        {
            x_prev[i] = x_prev[i - 1];
            v_prev[i] = v_prev[i - 1];
            t_prev[i] = t_prev[i - 1];
            
            xi_prev[i] = xi_prev[i - 1];
        }
        
        // Get Goal
        
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            g = GetMousePosition().y;
            
            ti = goal_time;
        }
        
        // Timed Spring
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 1.0f);
        GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "timer reset", TextFormat("%5.3f", goal_time), &goal_time, 0.0f, 3.0f);
        GuiSliderBar((Rectangle){ 100, 75, 120, 20 }, "apprehension", TextFormat("%5.3f", apprehension), &apprehension, 0.0f, 5.0f);
        
        GuiLabel((Rectangle){ 525, 20, 120, 20 }, TextFormat("Timer: %4.2f", ti));
        
        // Update Spring
        
        SetTargetFPS(1.0f / dt);
        
        t += dt;
        
        ti -= dt;
        ti = ti < 0.0f ? 0.0f : ti;
        
        timed_spring_damper_exact(x, v, xi, g, ti, halflife, dt);
        
        x_prev[0] = x;
        v_prev[0] = v;      
        t_prev[0] = t;
        
        xi_prev[0] = xi;
        
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawCircleV((Vector2){goalOffset, g}, 5, MAROON);
            DrawCircleV((Vector2){goalOffset, x}, 5, DARKBLUE);
            
            for (int i = 0; i < HISTORY_MAX - 1; i++)
            {
                Vector2 g_start = {goalOffset - (t - t_prev[i + 0]) * timescale, xi_prev[i + 0]};
                Vector2 g_stop  = {goalOffset - (t - t_prev[i + 1]) * timescale, xi_prev[i + 1]};
                
                DrawLineV(g_start, g_stop, MAROON);                
                DrawCircleV(g_start, 2, MAROON);
            }
            
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