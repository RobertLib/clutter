#include "camera.h"

void camera_init(Camera *c, float level_w)
{
    c->level_w = level_w;

    // start in the middle of the level, centered on screen
    c->scroll = level_w / 2.0f - SCREEN_W / 2.0f;
    if (c->scroll < 0.0f)
        c->scroll = 0.0f;
}

void camera_update(Camera *c, Player *p, float dt)
{
    (void)dt;

    // Convert player screen position to world position (center of sprite)
    const float world_cx = p->x + p->w * 0.5f + c->scroll;

    // Dead zone boundaries in screen space
    const float left_bound = SCREEN_W * 0.5f - CAMERA_DEADZONE_X;
    const float right_bound = SCREEN_W * 0.5f + CAMERA_DEADZONE_X;

    // Adjust scroll so player center stays within the dead zone
    float screen_cx = world_cx - c->scroll; // == p->x + p->w*0.5f (unchanged yet)
    if (screen_cx < left_bound)
        c->scroll = world_cx - left_bound;
    else if (screen_cx > right_bound)
        c->scroll = world_cx - right_bound;

    // Clamp scroll to world bounds [0, max_scroll]
    float max_scroll = c->level_w - SCREEN_W;
    if (max_scroll < 0.0f)
        max_scroll = 0.0f;
    if (c->scroll < 0.0f)
        c->scroll = 0.0f;
    if (c->scroll > max_scroll)
        c->scroll = max_scroll;

    // Reproject player back to screen space from world position + clamped scroll
    p->x = world_cx - p->w * 0.5f - c->scroll;

    // Hard screen clamps (reached only at world edges)
    if (p->x < 0.0f)
        p->x = 0.0f;
    if (p->x > SCREEN_W - p->w)
        p->x = SCREEN_W - p->w;
}
