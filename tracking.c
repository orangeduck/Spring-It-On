#include "common.h"

//--------------------------------------

void tracking_function(float& g, float& gv, float t, float freq, float amp, float phase, float off)
{
    g = amp * sin(t * freq + phase) + off;
    gv = amp * freq * cos(t * freq + phase);
}

void tracking_function1(float& g, float& gv, float t)
{
    tracking_function(g, gv, t, 2.0f * M_PI * 1.25, 74.0, 23.213123, 254);
}

void tracking_function2(float& g, float& gv, float t)
{
    tracking_function(g, gv, t, 2.0f * M_PI * 3.4, 28.0, 912.2381, 113);
}

//--------------------------------------

void tracking_spring_update(
    float& x,
    float& v,
    float x_goal,
    float v_goal,
    float a_goal,
    float x_gain,
    float v_gain,
    float a_gain,
    float dt)
{
    v = lerp(v, v + a_goal * dt, a_gain);
    v = lerp(v, v_goal, v_gain);
    v = lerp(v, (x_goal - x) / dt, x_gain);
    x = x + dt * v;
}

void tracking_spring_update_no_acceleration(
    float& x,
    float& v,
    float x_goal,
    float v_goal,
    float x_gain,
    float v_gain,
    float dt)
{
    v = lerp(v, v_goal, v_gain);
    v = lerp(v, (x_goal - x) / dt, x_gain);
    x = x + dt * v;
}

void tracking_spring_update_no_velocity_acceleration(
    float& x,
    float& v,
    float x_goal,
    float x_gain,
    float dt)
{
    v = lerp(v, (x_goal - x) / dt, x_gain);
    x = x + dt * v;
}

//--------------------------------------

void tracking_spring_update_improved(
    float& x,
    float& v,
    float x_goal,
    float v_goal,
    float a_goal,
    float x_halflife,
    float v_halflife,
    float a_halflife,
    float dt)
{
    v = damper_exact(v, v + a_goal * dt, a_halflife, dt);
    v = damper_exact(v, v_goal, v_halflife, dt);
    v = damper_exact(v, (x_goal - x) / dt, x_halflife, dt);
    x = x + dt * v;
}

void tracking_spring_update_no_acceleration_improved(
    float& x,
    float& v,
    float x_goal,
    float v_goal,
    float x_halflife,
    float v_halflife,
    float dt)
{
    v = damper_exact(v, v_goal, v_halflife, dt);
    v = damper_exact(v, (x_goal - x) / dt, x_halflife, dt);
    x = x + dt * v;
}

void tracking_spring_update_no_velocity_acceleration_improved(
    float& x,
    float& v,
    float x_goal,
    float x_halflife,
    float dt)
{
    v = damper_exact(v, (x_goal - x) / dt, x_halflife, dt);
    x = x + dt * v;
}

//--------------------------------------

void tracking_spring_update_exact(
    float& x,
    float& v,
    float x_goal,
    float v_goal,
    float a_goal,
    float x_gain,
    float v_gain,
    float a_gain,
    float dt,
    float gain_dt)
{
    float t0 = (1.0f - v_gain) * (1.0f - x_gain);
    float t1 = a_gain * (1.0f - v_gain) * (1.0f - x_gain);
    float t2 = (v_gain * (1.0f - x_gain)) / gain_dt;
    float t3 = x_gain / (gain_dt*gain_dt);
    
    float stiffness = t3;
    float damping = (1.0f - t0) / gain_dt;
    float spring_x_goal = x_goal;
    float spring_v_goal = (t2*v_goal + t1*a_goal) / ((1.0f - t0) / gain_dt);
    
    spring_damper_exact_stiffness_damping(
      x, 
      v, 
      spring_x_goal,
      spring_v_goal,
      stiffness,
      damping,
      dt);
}

void tracking_spring_update_no_acceleration_exact(
    float& x,
    float& v,
    float x_goal,
    float v_goal,
    float x_gain,
    float v_gain,
    float dt,
    float gain_dt)
{
    float t0 = (1.0f - v_gain) * (1.0f - x_gain);
    float t2 = (v_gain * (1.0f - x_gain)) / gain_dt;
    float t3 = x_gain / (gain_dt*gain_dt);
    
    float stiffness = t3;
    float damping = (1.0f - t0) / gain_dt;
    float spring_x_goal = x_goal;
    float spring_v_goal = t2*v_goal / ((1.0f - t0) / gain_dt);

    spring_damper_exact_stiffness_damping(
      x, 
      v, 
      spring_x_goal,
      spring_v_goal,
      stiffness,
      damping,
      dt);
}

void tracking_spring_update_no_velocity_acceleration_exact(
    float& x,
    float& v,
    float x_goal,
    float x_gain,
    float dt,
    float gain_dt)
{
    float t0 = 1.0f - x_gain;
    float t3 = x_gain / (gain_dt*gain_dt);
    
    float stiffness = t3;
    float damping = (1.0f - t0) / gain_dt;
    float spring_x_goal = x_goal;
    float spring_v_goal = 0.0f;
  
    spring_damper_exact_stiffness_damping(
      x, 
      v, 
      spring_x_goal,
      spring_v_goal,
      stiffness,
      damping,
      dt);
}

//--------------------------------------

float tracking_target_acceleration(
    float x_next,
    float x_curr,
    float x_prev,
    float dt)
{
    return (((x_next - x_curr) / dt) - ((x_curr - x_prev) / dt)) / dt;
}

float tracking_target_velocity(
    float x_next,
    float x_curr,
    float dt)
{
    return (x_next - x_curr) / dt;
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

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - tracking");

    // Init Variables

    float t = 0.0;
    float x = screenHeight / 2.0f;
    float v = 0.0;
    float g = x;
    float goalOffset = 600;

    float halflife = 0.1f;
    float dt = 1.0 / 60.0f;
    float timescale = 240.0f;
    
    float x_gain = 0.01f;
    float v_gain = 0.2f;
    float a_gain = 1.0f;
    
    float x_halflife = 1.0f;
    float v_halflife = 0.05f;
    float a_halflife = 0.0f;
    
    float v_max =   750.0f;
    float a_max = 12500.0f;
    
    bool tracking_toggle = true;
    int time_since_switch = 0;
    bool clamping = false;
    bool improved = true;
    bool exact = true;
    
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
        
        if (GuiButton((Rectangle){ 100, 45, 120, 20 }, "Transition"))
        {
            tracking_toggle = !tracking_toggle;
            time_since_switch = 0;
        }
        else
        {
            time_since_switch++;
        }
        
        //GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 1.0f);
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "dt", TextFormat("%5.3f", dt), &dt, 1.0 / 60.0f, 0.1f);
        
        // Update Spring
        
        SetTargetFPS(1.0f / dt);
        
        t += dt;
        
        float gv = 0.0f;
        if (tracking_toggle)
        {
            tracking_function1(g, gv, t);
        }
        else
        {
            tracking_function2(g, gv, t);
        }
        
        /*
        critical_spring_damper_exact(
          x, 
          v, 
          g, 
          gv, 
          halflife, 
          dt);
        */
        
        if (clamping || time_since_switch > 1)
        {
            float x_goal = g;
            float v_goal = tracking_target_velocity(g, g_prev[1], dt);
            float a_goal = tracking_target_acceleration(g, g_prev[1], g_prev[2], dt);
          
            if (clamping)
            {
                v_goal = clamp(v_goal, -v_max, v_max);
                a_goal = clamp(a_goal, -a_max, a_max);
            }
            
            if (exact)
            {
                tracking_spring_update_exact(
                    x, v,
                    x_goal, v_goal, a_goal,
                    x_gain, v_gain, a_gain,
                    dt,
                    1.0f / 60.0f);
            }
            else if (improved)
            {
                tracking_spring_update_improved(
                    x, v,
                    x_goal, v_goal, a_goal,
                    x_halflife, v_halflife, a_halflife,
                    dt);
            }
            else
            {
                tracking_spring_update(
                    x, v,
                    x_goal, v_goal, a_goal,
                    x_gain, v_gain, a_gain,
                    dt);
            }
        }
        else if (time_since_switch > 0)
        {
            float x_goal = g;
            float v_goal = tracking_target_velocity(g, g_prev[1], dt);
            
            if (exact)
            {
                tracking_spring_update_no_acceleration_exact(
                    x, v,
                    x_goal, v_goal,
                    x_gain, v_gain,
                    dt,
                    1.0f / 60.0f);
            }
            else if (improved)
            {
                tracking_spring_update_no_acceleration_improved(
                    x, v,
                    x_goal, v_goal,
                    x_halflife, v_halflife,
                    dt);
            }
            else
            {
                tracking_spring_update_no_acceleration(
                    x, v,
                    x_goal, v_goal,
                    x_gain, v_gain,
                    dt);
            }
        }
        else
        {
            float x_goal = g;
            
            if (exact)
            {
                tracking_spring_update_no_velocity_acceleration_exact(
                    x, v,
                    x_goal, 
                    x_gain,
                    dt,
                    1.0f / 60.0f);
            }
            else if (improved)
            {
                tracking_spring_update_no_velocity_acceleration_improved(
                    x, v,
                    x_goal, 
                    x_halflife,
                    dt);
            }
            else
            {
                tracking_spring_update_no_velocity_acceleration(
                    x, v,
                    x_goal, 
                    x_gain,
                    dt);
            }
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