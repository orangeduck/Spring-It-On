#include "common.h"

// Inspired by: https://openprocessing.org/sketch/1684527

enum
{
    HISTORY_MAX = 256
};

float x_prev[HISTORY_MAX];
float t_prev[HISTORY_MAX];

// Scaled smoothstep including negative section
float smoothstep(float t, float s)
{
    return s * (t > 1.0f ? 1.0f : 3*t*t - 2*t*t*t);
}

// Scaled smoothstep derivative including negative section
float smoothstep_dt(float t, float s)
{
    return s * (t > 1.0f ? 0.0f : 6*t - 6*t*t);
}

// Solve for smoothstep parameters `t` and `s` given new target `x` and velocity `v`
void smoothstep_solve(
    float& t, float& s, float x, float v, float overshoot = 0.05f, float eps=1e-8f)
{
    // If velocity is zero start from time zero with scale to match `x`
    if (fabsf(v) < 1e-8f)
    {
        t = 0.0f;
        s = x;
        return;
    }
    
    // If `x` is negative then invert signs and re-solve
    if (x < 0.0)
    {
        smoothstep_solve(t, s, -x, -v);
        s = -s;
        return;
    }
    
    // Find the possible times that solve for the given `x` and `v`
    float t0 = (v - 6*x + sqrtf(max((6*x - v)*(6*x - v) + 8*v*v, eps))) / (4*v);
    float t1 = (v - 6*x - sqrtf(max((6*x - v)*(6*x - v) + 8*v*v, eps))) / (4*v);
    
    // Overshoot if the alternative time is between -0.5f and -(0.5f + overshoot)
    t = -0.5f > t1 && t1 > -(0.5f + overshoot) ? t1 : t0;

    // Find the un-scaled velocity at the fitted time
    float vt = smoothstep_dt(t, 1.0f);        

    // Find the scaling factor so that the velocity at the fitted time matches
    s = fabsf(vt) < eps ? 0.0f : v / vt;
}

int main(void)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - inertial easing");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float g = x;
    float goalOffset = 400;

    float blendtime = 1.0f;
    float overshoot = 0.05f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float et = 0.0f;
    float es = 0.0f;
    float eo = x;

    bool draw_axis = false;
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
        GuiSliderBar((Rectangle){ 100, 50, 120, 20 }, "overshoot", TextFormat("%5.3f", overshoot), &overshoot, 0.0f, 0.5f);
        GuiDrawText(TextFormat("t=% 5.2f", et), (Rectangle){ 275, 20, 120, 20 }, TEXT_ALIGN_LEFT, DARKGRAY);
        GuiDrawText(TextFormat("s=% 5.2f", es), (Rectangle){ 325, 20, 120, 20 }, TEXT_ALIGN_LEFT, DARKGRAY);
        GuiCheckBox((Rectangle){ 400, 20, 20, 20 }, "draw fit", &draw_fit);
        GuiCheckBox((Rectangle){ 500, 20, 20, 20 }, "draw axis", &draw_axis);
        
        // Check if target changed
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            if (g != GetMousePosition().y)
            {
                // Update Target
                g = GetMousePosition().y;
                
                // Solve for smoothstep parameters `s` and `t`
                smoothstep_solve(et, es, g - x, smoothstep_dt(et, es), overshoot);
                
                // Set the offset to the difference between the target and current eased value
                eo = x - smoothstep(et, es);
            }
        }

        // Update the easing time
        et += dt / max(blendtime, 1e-4f);
        
        // Update the eased value
        x = eo + smoothstep(et, es);
        
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
                    
                    float py0 = eo + smoothstep(et + pt0, es);
                    float py1 = eo + smoothstep(et + pt1, es);
                    
                    Vector2 p_start = { goalOffset + pt0 * timescale, py0 };
                    Vector2 p_stop = { goalOffset + pt1 * timescale, py1 };
                    
                    DrawLineV(p_start, p_stop, PURPLE);
                }     
            }

            if (draw_axis)
            {
                Vector2 yaxis_start = { goalOffset - et * timescale, 0 };
                Vector2 yaxis_stop = { goalOffset - et * timescale, screenHeight };
                DrawLineV(yaxis_start, yaxis_stop, Fade(GRAY, 0.5f));
                
                Vector2 laxis_start = { 0, eo + smoothstep(0.0, es) };
                Vector2 laxis_stop = { screenWidth, eo + smoothstep(0.0, es) };
                DrawLineV(laxis_start, laxis_stop, Fade(GRAY, 0.5f));

                Vector2 uaxis_start = { 0, eo + smoothstep(1.0, es) };
                Vector2 uaxis_stop = { screenWidth, eo + smoothstep(1.0, es) };
                DrawLineV(uaxis_start, uaxis_stop, Fade(GRAY, 0.5f));
            }
            
        EndDrawing();
        
    }

    CloseWindow();

    return 0;
}