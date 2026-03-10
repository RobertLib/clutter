#include "bullet.h"
#include "constants.h"

static Bullet bullets[MAX_BULLETS];

void bullets_init(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = false;
}

void bullets_spawn(float x, float y)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
        {
            bullets[i].active = true;
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].vx = BULLET_SPEED;
            return;
        }
    }
}

void bullets_update(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
            continue;

        bullets[i].x += bullets[i].vx * dt;

        if (bullets[i].x > (float)SCREEN_W)
            bullets[i].active = false;
    }
}

void bullets_render(SDL_Renderer *r)
{
    SDL_SetRenderDrawColor(r, 255, 0, 0, 255);

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
            continue;

        SDL_FRect rect = {bullets[i].x, bullets[i].y, BULLET_W, BULLET_H};
        SDL_RenderFillRect(r, &rect);
    }
}

bool bullets_check_hit(float x, float y, float w, float h)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
            continue;

        if (bullets[i].x < x + w &&
            bullets[i].x + BULLET_W > x &&
            bullets[i].y < y + h &&
            bullets[i].y + BULLET_H > y)
        {
            bullets[i].active = false;
            return true;
        }
    }
    return false;
}
