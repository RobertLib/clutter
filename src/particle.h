#pragma once

#include <SDL3/SDL.h>

#define MAX_PARTICLES 2000

typedef enum
{
    PARTICLE_FIRE,
    PARTICLE_EXPLOSION,
} ParticleType;

void particles_init(void);
void particles_update(float dt);
void particles_render(SDL_Renderer *r, float scroll);
void particles_emit(float world_x, float world_y, ParticleType type, Uint8 br, Uint8 bg, Uint8 bb);
