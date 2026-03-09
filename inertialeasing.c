#include "common.h"

enum
{
    HISTORY_MAX = 256
};

float x_prev[HISTORY_MAX];
float t_prev[HISTORY_MAX];

float cubic_easing(float t)
{
    // We allow it to go less than zero
    t = min(t, 1.0f);
    return 3*t*t - 2*t*t*t;
}

float cubic_easing_dt(float t)
{
    t = min(t, 1.0f);
    return 6*t - 6*t*t;
}

float cubic_easing_vmax() { return 1.5f; }              // Maximum velocity
float cubic_easing_vmax_time() { return 0.5f; }         // Time at maximum velocity
float cubic_easing_amin() { return -6.0f; }             // Minimum acceleration
float cubic_easing_tmin() { return -sqrtf(1.0f/3.0f); } // Time where f(t) = 1 and t < 0

float cubic_easing_minimize(float x, float v, float lower, float upper, float eps = 0.000001f)
{
    float phi = (1.0f + sqrtf(5.0f)) / 2.0f;
    
    int iterations = 1000;
    
    for (int i = 0; i < iterations; i++)
    {
        float xa = lerp(lower, upper, 1.0f / (phi + 1.0f));
        float xb = lerp(lower, xa, 1.0f + (1.0f / phi));
        
        float vta = cubic_easing_dt(xa);
        float sa = v / vta;
        float fa = fabsf((sa * sa * sign(sa) * (1.0f - cubic_easing(xa))) - x);
        
        float vtb = cubic_easing_dt(xb);
        float sb = v / vtb;
        float fb = fabsf((sb * sb * sign(sb) * (1.0f - cubic_easing(xb))) - x);
        
        if (fa < fb)
        {
            upper = xb;
        }
        else
        {
            lower = xa;
        }
        
        if (upper - lower < eps) { break; }
    }
    
    return (lower + upper) / 2.0f;
}

void cubic_easing_solve(float& t, float& s, float x, float v)
{
    // If velocity is zero start from time zero
    if (v == 0.0f)
    {
        t = 0.0f;
        s = sqrtf(fabsf(x)) * sign(x);
        return;
    }
    
    // If x is negative then invert signs and re-solve
    if (x < 0.0)
    {
        cubic_easing_solve(t, s, -x, -v);
        s = -s;
        return;
    }
    
    // x is now positive and v is non-zero.
    
    // If v is in the opposite direction to x we start the time at below zero.
    // Find the best fitting time from below zero and where f(t) = 1 (where t < 0)
    if (v < 0)
    {
        t = cubic_easing_minimize(x, v, cubic_easing_tmin(), 0.0);
        s = v / cubic_easing_dt(t); // Re-scale so the velocity matches
        return;
    }
    
    // Check if x is less than the minimum distance travelled at current speed 
    // `v` when decelerating continuously at minimum acceleration. This gives us 
    // the case where we must over-shoot. 
    if (x < 0.5f * (v*v) / -cubic_easing_amin())
    {
        t = cubic_easing_minimize(x, v, -100.0f, cubic_easing_tmin());
        s = v / cubic_easing_dt(t); // Re-scale so the velocity matches
        return;
    }
    
    // When the easing function is scaled by s^2 the derivative is scaled by 
    // `s`. Thus the maximum velocity after the scaling is `vmax * s`.
    //
    // The maximum velocity must be more than the current velocity
    // so the minimum scaling factor `smin` is obtained by `vmax * smin = v`.
    
    float smin = v / cubic_easing_vmax();
    
    if (smin * smin * (1.0f - cubic_easing_vmax()) < x)
    {
        // Search bottom section of easing function
        t = cubic_easing_minimize(x, v, 0.0f, cubic_easing_vmax_time());
        s = v / cubic_easing_dt(t);
        return;
    }
    else
    {
        // Search top section of easing function
        t = cubic_easing_minimize(x, v, cubic_easing_vmax_time(), 1.0f);
        s = v / cubic_easing_dt(t); // Re-scale so the velocity matches
        return;
    }
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
    float goalOffset = 600;

    float blendtime = 0.1f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float ease_t = 0.0f;
    float ease_s = 0.0f;
    float ease_o = x;

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

        // Get Goal
        
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            float g_new = GetMousePosition().y;
            if (g != g_new)
            {
                g = g_new;
                cubic_easing_solve(ease_t, ease_s, g - x, ease_s * cubic_easing_dt(ease_t));
                ease_o = x - ease_s*ease_s*sign(ease_s) * cubic_easing(ease_t);
                printf("Solve %f!\n", ease_s);
            }
        }
        
        // Inertial Easing
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "blendtime", TextFormat("%5.3f", blendtime), &blendtime, 0.0f, 1.0f);
        //GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);

        // Update Spring
        
        //SetTargetFPS(1.0f / dt);
        
        t += dt;

        // Update Inertial Easing
        
        ease_t = min(ease_t + dt / fabsf(ease_s) / blendtime, 1.0f);
        x = ease_o + ease_s*ease_s*sign(ease_s) * cubic_easing(ease_t);
        
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

            
        EndDrawing();
        
    }

    CloseWindow();

    return 0;
}