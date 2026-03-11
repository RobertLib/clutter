#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>

#define MAX_BULLETS 128
#define BULLET_SPEED 400.0f
#define BULLET_W 10.0f
#define BULLET_H 4.0f

typedef struct
{
    float x, y;
    float vx;
    bool active;

} Bullet;

void bullets_init(void);
void bullets_spawn(float x, float y, float vx);
void bullets_update(float dt);
void bullets_render(SDL_Renderer *r);
bool bullets_check_hit(float x, float y, float w, float h);
