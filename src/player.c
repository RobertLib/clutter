#include "player.h"
#include <math.h>

void player_init(Player *p, float x, float y)
{
    p->x = x;
    p->y = y;
    p->w = PLAYER_W;
    p->h = PLAYER_H;
    p->vx = PLANE_FORWARD_SPEED;
    p->vy = 0.0f;
    p->angle = 0.0f;
    p->hp = PLAYER_MAX_HP;
    timer_clear(&p->hit_cooldown);
    p->facing = 1;
    p->is_dying = false;
    timer_clear(&p->dying_timer);
}

void player_update(Player *p, float dt)
{
    if (p->is_dying)
        return;

    const bool *k = SDL_GetKeyboardState(NULL);
    if (!k)
        return;

    timer_tick(&p->hit_cooldown, dt);

    // Plane always flies forward; LEFT/RIGHT change direction with inertia
    float target_vx = PLANE_FORWARD_SPEED * (float)p->facing;
    if (k[SDL_SCANCODE_LEFT])
    {
        target_vx = -PLANE_FORWARD_SPEED;
        p->facing = -1;
    }
    if (k[SDL_SCANCODE_RIGHT])
    {
        target_vx = PLANE_FORWARD_SPEED;
        p->facing = 1;
    }
    p->vx += (target_vx - p->vx) * 6.0f * dt;

    // Vertical: thrust + gravity + drag
    if (k[SDL_SCANCODE_UP])
        p->vy -= PLANE_VY_ACCEL * dt;
    if (k[SDL_SCANCODE_DOWN])
        p->vy += PLANE_VY_ACCEL * dt;

    p->vy += PLANE_GRAVITY * dt;
    p->vy -= p->vy * PLANE_VY_DRAG * dt;

    if (p->vy > PLANE_MAX_VY)
        p->vy = PLANE_MAX_VY;
    if (p->vy < -PLANE_MAX_VY)
        p->vy = -PLANE_MAX_VY;

    p->x += p->vx * dt;
    p->y += p->vy * dt;

    // Visual pitch angle derived from velocity
    p->angle = atan2f(p->vy, fabsf(p->vx) + 1.0f);
    if (p->angle > PLANE_MAX_ANGLE)
        p->angle = PLANE_MAX_ANGLE;
    if (p->angle < -PLANE_MAX_ANGLE)
        p->angle = -PLANE_MAX_ANGLE;

    // Vertical screen clamp
    if (p->y < 0.0f)
    {
        p->y = 0.0f;
        if (p->vy < 0.0f)
            p->vy = 0.0f;
    }
    if (p->y > (float)(SCREEN_H - PLAYER_H))
    {
        p->y = (float)(SCREEN_H - PLAYER_H);
        if (p->vy > 0.0f)
            p->vy = 0.0f;
    }
}

void player_render(Player *p, SDL_Renderer *r)
{
    if (p->is_dying)
        return;

    // blink during invincibility
    if (timer_running(&p->hit_cooldown) && (int)(p->hit_cooldown.t * 10.0f) % 2 == 0)
        return;

    // Draw rotated rectangle using two triangles
    float cx = p->x + p->w * 0.5f;
    float cy = p->y + p->h * 0.5f;
    float a = p->angle * (float)p->facing;
    float sa = sinf(a);
    float ca = cosf(a);
    float hw = p->w * 0.5f;
    float hh = p->h * 0.5f;

    // Rotate all 4 corners around center
#define RX(lx, ly) (cx + (lx) * ca - (ly) * sa)
#define RY(lx, ly) (cy + (lx) * sa + (ly) * ca)
    SDL_FColor col = {1.0f, 220 / 255.f, 0.0f, 1.0f};
    SDL_Vertex verts[4] = {
        {{RX(-hw, -hh), RY(-hw, -hh)}, col, {0, 0}},
        {{RX(hw, -hh), RY(hw, -hh)}, col, {0, 0}},
        {{RX(hw, hh), RY(hw, hh)}, col, {0, 0}},
        {{RX(-hw, hh), RY(-hw, hh)}, col, {0, 0}},
    };
#undef RX
#undef RY
    int idx[] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(r, NULL, verts, 4, idx, 6);
}

void player_take_hit(Player *p)
{
    if (timer_running(&p->hit_cooldown))
        return;
    if (p->hp > 0)
        p->hp--;
    timer_set(&p->hit_cooldown, 1.5f);
}

bool player_is_alive(const Player *p)
{
    return p->hp > 0;
}

void player_terrain_death(Player *p)
{
    if (p->is_dying)
        return;
    p->is_dying = true;
    timer_set(&p->dying_timer, 1.5f);
    if (p->hp > 0)
        p->hp--;
}
