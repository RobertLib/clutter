#include "enemy.h"
#include "bullet.h"

static Enemy enemies[MAX_ENEMIES];

void enemies_init(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
}

void enemies_spawn(float x, float y)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].active = true;
            enemies[i].x = x;
            enemies[i].y = y;
            enemies[i].vx = -ENEMY_SPEED;
            return;
        }
    }
}

void enemies_update(float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
            continue;

        enemies[i].x += enemies[i].vx * dt;

        if (enemies[i].x < -200.0f)
            enemies[i].active = false;
    }
}

void enemies_render(SDL_Renderer *r, float cameraX)
{
    SDL_SetRenderDrawColor(r, 0, 255, 0, 255);

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
            continue;

        SDL_FRect rect = {
            enemies[i].x - cameraX,
            enemies[i].y,
            ENEMY_W,
            ENEMY_H};

        SDL_RenderFillRect(r, &rect);
    }
}

// Returns the number of enemies killed by bullets (scroll converts world->screen).
int enemies_check_bullet_collisions(float scroll)
{
    int killed = 0;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
            continue;

        float sx = enemies[i].x - scroll;
        if (bullets_check_hit(sx, enemies[i].y, ENEMY_W, ENEMY_H))
        {
            enemies[i].active = false;
            killed++;
        }
    }

    return killed;
}

// Returns true if any enemy overlaps the given rectangle (screen coords).
bool enemies_collide_rect(float x, float y, float w, float h, float scroll)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
            continue;

        float sx = enemies[i].x - scroll;
        if (sx < x + w &&
            sx + ENEMY_W > x &&
            enemies[i].y < y + h &&
            enemies[i].y + ENEMY_H > y)
        {
            enemies[i].active = false;
            return true;
        }
    }
    return false;
}
