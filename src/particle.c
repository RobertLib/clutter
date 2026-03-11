#include "particle.h"
#include "constants.h"
#include <stdlib.h>

typedef struct
{
    float x, y; /* world-space */
    float vx, vy;
    float life;
    float max_life;
    float size;
    bool active;
} Particle;

static Particle g_particles[MAX_PARTICLES];

static float rand_range(float lo, float hi)
{
    return lo + (float)rand() / (float)RAND_MAX * (hi - lo);
}

void particles_init(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
        g_particles[i].active = false;
}

void particles_update(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &g_particles[i];
        if (!p->active)
            continue;

        p->life -= dt;
        if (p->life <= 0.0f)
        {
            p->active = false;
            continue;
        }

        p->x += p->vx * dt;
        p->y += p->vy * dt;

        /* Fire slows down as it rises – slight upward drag */
        p->vy += 35.0f * dt;
    }
}

void particles_render(SDL_Renderer *r, float scroll)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &g_particles[i];
        if (!p->active)
            continue;

        /* t: 0 = just born, 1 = about to die */
        float t = 1.0f - (p->life / p->max_life);

        Uint8 rc, gc, bc, alpha;

        if (t < 0.5f)
        {
            float u = t / 0.5f; /* 0 -> 1 over first half */
            rc = 255;
            gc = (Uint8)(200 - u * 100); /* yellow -> orange */
            bc = 0;
            alpha = 220;
        }
        else
        {
            float u = (t - 0.5f) / 0.5f; /* 0 -> 1 over second half */
            rc = 255;
            gc = (Uint8)(100 * (1.0f - u)); /* orange -> red */
            bc = 0;
            alpha = (Uint8)(220 * (1.0f - u)); /* fade out */
        }

        float size = p->size * (1.0f - t * 0.6f);
        if (size < 1.0f)
            size = 1.0f;

        float sx = p->x - scroll;
        float sy = p->y;

        /* Skip particles clearly off-screen */
        if (sx < -size || sx > (float)SCREEN_W + size)
            continue;

        SDL_SetRenderDrawColor(r, rc, gc, bc, alpha);
        SDL_FRect rect = {sx - size * 0.5f, sy - size * 0.5f, size, size};
        SDL_RenderFillRect(r, &rect);
    }

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

void particles_emit(float world_x, float world_y, ParticleType type)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &g_particles[i];
        if (p->active)
            continue;

        p->active = true;
        p->x = world_x;
        p->y = world_y;

        if (type == PARTICLE_FIRE)
        {
            p->vx = rand_range(-18.0f, 18.0f);
            p->vy = rand_range(-110.0f, -55.0f);
            p->max_life = rand_range(0.5f, 1.3f);
            p->life = p->max_life;
            p->size = rand_range(4.0f, 9.0f);
        }

        return;
    }
    /* Pool exhausted – silently drop */
}
