#include "common.h"
#include <float.h>

//--------------------------------------

void piecewise_interpolation(
    float& x,
    float& v,
    float t,
    float pnts[],
    int npnts)
{
    t = t * (npnts - 1);
    int i0 = floorf(t);
    int i1 = i0 + 1;
    i0 = i0 > npnts - 1 ? npnts - 1 : i0;
    i1 = i1 > npnts - 1 ? npnts - 1 : i1;
    float alpha = fmod(t, 1.0f);
    
    x = lerp(pnts[i0], pnts[i1], alpha);
    v = (pnts[i0] - pnts[i1]) / npnts;
}

//--------------------------------------

enum
{
    CTRL_MAX = 8,
};

float ctrlx[CTRL_MAX];
float ctrly[CTRL_MAX];

int main(void)
{
    // Init Window
    
    const int screenWidth = 640;
    const int screenHeight = 360;

    InitWindow(screenWidth, screenHeight, "raylib [springs] example - interpolation");

    // Init Variables

    float halflife = 0.5f;
    float frequency = 1.5f;
    int ctrl_selected = -1;

    for (int i = 0; i < CTRL_MAX; i++)
    {
        ctrlx[i] = ((float)i / CTRL_MAX) * 600 + 20;
        ctrly[i] = sinf((float)i) * 100 + 100;
    }

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            float best_dist = FLT_MAX;
            for (int i = 0; i < CTRL_MAX; i++)
            {
                float dist = squaref(ctrlx[i] - GetMousePosition().x) + squaref(ctrly[i] - GetMousePosition().y);
                if (dist < best_dist)
                {
                    best_dist = dist;
                    ctrl_selected = i;
                }
            }
        }
        
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            ctrlx[ctrl_selected] = GetMousePosition().x;
            ctrly[ctrl_selected] = GetMousePosition().y;
        }
        
        // Interpolation
        
        GuiSliderBar((Rectangle){ 100, 20, 120, 20 }, "frequency", TextFormat("%5.3f", frequency), &frequency, 0.0f, 3.0f);
        GuiSliderBar((Rectangle){ 100, 45, 120, 20 }, "halflife", TextFormat("%5.3f", halflife), &halflife, 0.0f, 1.0f);
        
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            for (int i = 0; i < CTRL_MAX; i++)
            {
               DrawCircleV((Vector2){ctrlx[i], ctrly[i]}, 4, MAROON); 
            }
            
            for (int i = 0; i < CTRL_MAX - 1; i++)
            {
               DrawLineV(
                   (Vector2){ctrlx[i], ctrly[i]}, 
                   (Vector2){ctrlx[i + 1], ctrly[i + 1]}, RED); 
            }
            
            float sx = ctrlx[0];
            float sy = ctrly[0];
            float svx = (ctrlx[1] - ctrlx[0]) / CTRL_MAX;
            float svy = (ctrly[1] - ctrly[0]) / CTRL_MAX;
            
            DrawCircleV((Vector2){sx, sy}, 2, BLUE);
            
            int subsamples = 100;
            for (int i = 0; i < subsamples; i++)
            {
               Vector2 start = {sx, sy};
               
               float goalx, goaly;
               float goalvx, goalvy;
               
               piecewise_interpolation(goalx, goalvx, (float)i / (subsamples - 1), ctrlx, CTRL_MAX);
               piecewise_interpolation(goaly, goalvy, (float)i / (subsamples - 1), ctrly, CTRL_MAX);
               
               spring_damper_exact(sx, svx, goalx, goalvx, halflife, frequency, (float)CTRL_MAX / subsamples);
               spring_damper_exact(sy, svy, goaly, goalvy, halflife, frequency, (float)CTRL_MAX / subsamples);
               
               Vector2 stop = {sx, sy};
               
               DrawLineV(start, stop, DARKBLUE); 
               
               DrawCircleV(stop, 2, BLUE);
            }
            
        EndDrawing();
        
    }

    CloseWindow();

    return 0;
}