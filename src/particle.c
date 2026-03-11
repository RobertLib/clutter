#include "particle.h"
#include "constants.h"
#include <stdlib.h>
#include <math.h>

typedef struct
{
    float x, y; /* world-space */
    float vx, vy;
    float life;
    float max_life;
    float size;
    bool active;
    ParticleType type;
    Uint8 base_r, base_g, base_b;
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

        if (p->type == PARTICLE_EXPLOSION)
        {
            /* Explosion falls with gravity and loses horizontal speed */
            p->vy += 200.0f * dt;
            p->vx *= (1.0f - 3.0f * dt);
        }
        else
        {
            /* Fire slows down as it rises – slight upward drag */
            p->vy += 35.0f * dt;
        }
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

        if (p->type == PARTICLE_EXPLOSION)
        {
            /* Fade from white flash through base color to transparent */
            float br = (float)p->base_r / 255.0f;
            float bg = (float)p->base_g / 255.0f;
            float bb = (float)p->base_b / 255.0f;
            if (t < 0.25f)
            {
                float u = t / 0.25f;
                rc = (Uint8)(255 * (1.0f - u * (1.0f - br)));
                gc = (Uint8)(255 * (1.0f - u * (1.0f - bg)));
                bc = (Uint8)(255 * (1.0f - u * (1.0f - bb)));
                alpha = 255;
            }
            else
            {
                float u = (t - 0.25f) / 0.75f;
                rc = (Uint8)(p->base_r * (1.0f - u));
                gc = (Uint8)(p->base_g * (1.0f - u));
                bc = (Uint8)(p->base_b * (1.0f - u));
                alpha = (Uint8)(255 * (1.0f - u));
            }
        }
        else if (t < 0.5f)
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

void particles_emit(float world_x, float world_y, ParticleType type, Uint8 br, Uint8 bg, Uint8 bb)
{
    if (type == PARTICLE_EXPLOSION)
    {
        /* Burst of 40 particles in random directions */
        int spawned = 0;
        for (int i = 0; i < MAX_PARTICLES && spawned < 40; i++)
        {
            Particle *p = &g_particles[i];
            if (p->active)
                continue;

            float angle = rand_range(0.0f, 6.2832f);
            float speed = rand_range(60.0f, 260.0f);
            p->x = world_x;
            p->y = world_y;
            p->vx = cosf(angle) * speed;
            p->vy = sinf(angle) * speed;
            p->max_life = rand_range(0.3f, 0.8f);
            p->life = p->max_life;
            p->size = rand_range(4.0f, 12.0f);
            p->type = PARTICLE_EXPLOSION;
            p->base_r = br;
            p->base_g = bg;
            p->base_b = bb;
            p->active = true;
            spawned++;
        }
        return;
    }

    /* PARTICLE_FIRE – single particle, color params ignored */
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &g_particles[i];
        if (p->active)
            continue;

        p->active = true;
        p->x = world_x;
        p->y = world_y;
        p->vx = rand_range(-18.0f, 18.0f);
        p->vy = rand_range(-110.0f, -55.0f);
        p->max_life = rand_range(0.5f, 1.3f);
        p->life = p->max_life;
        p->size = rand_range(4.0f, 9.0f);
        p->type = PARTICLE_FIRE;
        return;
    }
    /* Pool exhausted – silently drop */
}
