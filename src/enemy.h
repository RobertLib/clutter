#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>

#define MAX_ENEMIES 64
#define ENEMY_SPEED 100.0f
#define ENEMY_W 28.0f
#define ENEMY_H 28.0f

typedef struct
{
    float x, y;
    float vx;
    bool active;

} Enemy;

void enemies_init(void);
void enemies_spawn(float x, float y, float vx);
void enemies_update(float dt, float scroll);
void enemies_render(SDL_Renderer *r, float scroll);
int enemies_check_bullet_collisions(float scroll);
bool enemies_collide_rect(float x, float y, float w, float h, float scroll);
