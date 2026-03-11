#include "player.h"

void player_init(Player *p, float x, float y)
{
    p->x = x;
    p->y = y;
    p->w = PLAYER_W;
    p->h = PLAYER_H;
    p->hp = PLAYER_MAX_HP;
    p->hit_cooldown = 0.0f;
    p->facing = 1;
}

void player_update(Player *p, float dt)
{
    const bool *k = SDL_GetKeyboardState(NULL);
    if (!k)
        return;

    if (p->hit_cooldown > 0.0f)
        p->hit_cooldown -= dt;

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
    // blink during invincibility
    if (p->hit_cooldown > 0.0f && (int)(p->hit_cooldown * 10.0f) % 2 == 0)
        return;

    SDL_FRect rect = {p->x, p->y, p->w, p->h};
    SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
    SDL_RenderFillRect(r, &rect);
}

void player_take_hit(Player *p)
{
    if (p->hit_cooldown > 0.0f)
        return;
    if (p->hp > 0)
        p->hp--;
    p->hit_cooldown = 1.5f;
}

bool player_is_alive(const Player *p)
{
    return p->hp > 0;
}
