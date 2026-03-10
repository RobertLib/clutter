#pragma once

#include <SDL3/SDL.h>
#include "constants.h"

#define PLAYER_SPEED 200.0f
#define PLAYER_W 32
#define PLAYER_H 32
#define PLAYER_MAX_HP 3

typedef struct
{
    float x, y;
    float w, h;
    int hp;
    float hit_cooldown;

} Player;

void player_init(Player *p, float x, float y);
void player_update(Player *p, float dt);
void player_render(Player *p, SDL_Renderer *r);
void player_take_hit(Player *p);
bool player_is_alive(const Player *p);
