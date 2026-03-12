#pragma once

#include <SDL3/SDL.h>
#include "constants.h"
#include "timer.h"

#define PLAYER_W 44
#define PLAYER_H 14
#define PLAYER_MAX_HP 3

// Plane physics
#define PLANE_FORWARD_SPEED 175.0f // horizontal cruise speed (px/s)
#define PLANE_VY_ACCEL 320.0f      // vertical thrust (px/s²)
#define PLANE_GRAVITY 70.0f        // gravity (px/s²)
#define PLANE_VY_DRAG 3.5f         // aerodynamic drag coefficient
#define PLANE_MAX_VY 240.0f        // max vertical speed (px/s)
#define PLANE_MAX_ANGLE 0.42f      // max visual pitch (~24°)

typedef struct
{
    float x, y;
    float w, h;
    float vx, vy; // velocity
    float angle;  // visual pitch in radians
    int hp;
    Timer hit_cooldown;
    int facing; // +1 = right, -1 = left
    bool is_dying;
    Timer dying_timer;

} Player;

void player_init(Player *p, float x, float y);
void player_update(Player *p, float dt);
void player_render(Player *p, SDL_Renderer *r);
void player_take_hit(Player *p);
bool player_is_alive(const Player *p);
void player_terrain_death(Player *p);
