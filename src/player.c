#include "player.h"

void player_init(Player *p, float x, float y)
{
    p->x = x;
    p->y = y;
    p->w = PLAYER_W;
    p->h = PLAYER_H;
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

    if (k[SDL_SCANCODE_LEFT])
    {
        p->x -= PLAYER_SPEED * dt;
        p->facing = -1;
    }

    if (k[SDL_SCANCODE_RIGHT])
    {
        p->x += PLAYER_SPEED * dt;
        p->facing = 1;
    }

    if (k[SDL_SCANCODE_UP])
        p->y -= PLAYER_SPEED * dt;

    if (k[SDL_SCANCODE_DOWN])
        p->y += PLAYER_SPEED * dt;

    // clamp to screen bounds
    if (p->y < 0.0f)
        p->y = 0.0f;
    if (p->y > (float)(SCREEN_H - PLAYER_H))
        p->y = (float)(SCREEN_H - PLAYER_H);
}

void player_render(Player *p, SDL_Renderer *r)
{
    if (p->is_dying)
        return;

    // blink during invincibility
    if (timer_running(&p->hit_cooldown) && (int)(p->hit_cooldown.t * 10.0f) % 2 == 0)
        return;

    SDL_FRect rect = {p->x, p->y, p->w, p->h};
    SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
    SDL_RenderFillRect(r, &rect);
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
