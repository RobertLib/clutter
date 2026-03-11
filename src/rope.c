#include "rope.h"
#include <math.h>

#define ROPE_GRAVITY 600.0f // downward acceleration (px/s²)
#define ROPE_DAMPING 0.98f  // velocity damping to avoid endless oscillation

void rope_init(Rope *r, float anchor_x, float anchor_y)
{
    // Place all nodes in a vertical line hanging from the anchor
    for (int i = 0; i <= ROPE_SEGMENTS; i++)
    {
        float ny = anchor_y + i * ROPE_SEG_LEN;
        r->nodes[i].x = anchor_x;
        r->nodes[i].y = ny;
        r->nodes[i].px = anchor_x;
        r->nodes[i].py = ny;
    }
}

void rope_update(Rope *r, float anchor_x, float anchor_y, float dt)
{
    float dt2 = dt * dt;

    // --- 1. Pin anchor to the bottom of the plane ---
    r->nodes[0].x = anchor_x;
    r->nodes[0].y = anchor_y;
    r->nodes[0].px = anchor_x;
    r->nodes[0].py = anchor_y;

    // --- 2. Verlet integration for all free nodes ---
    for (int i = 1; i <= ROPE_SEGMENTS; i++)
    {
        RopeNode *n = &r->nodes[i];

        float vx = (n->x - n->px) * ROPE_DAMPING;
        float vy = (n->y - n->py) * ROPE_DAMPING;

        float nx = n->x + vx;
        float ny = n->y + vy + ROPE_GRAVITY * dt2;

        n->px = n->x;
        n->py = n->y;
        n->x = nx;
        n->y = ny;
    }

    // --- 3. Solve length constraints (relaxation) ---
    for (int iter = 0; iter < ROPE_ITER; iter++)
    {
        for (int i = 0; i < ROPE_SEGMENTS; i++)
        {
            RopeNode *a = &r->nodes[i];
            RopeNode *b = &r->nodes[i + 1];

            float dx = b->x - a->x;
            float dy = b->y - a->y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 0.0001f)
                continue;

            float diff = (dist - ROPE_SEG_LEN) / dist;

            if (i == 0)
            {
                // Anchor is fixed – move only the second node
                b->x -= dx * diff;
                b->y -= dy * diff;
            }
            else
            {
                // Both nodes are free – split the correction equally
                b->x -= dx * diff * 0.5f;
                b->y -= dy * diff * 0.5f;
                a->x += dx * diff * 0.5f;
                a->y += dy * diff * 0.5f;
            }
        }
    }
}

void rope_render(const Rope *r, SDL_Renderer *renderer)
{
    // --- Draw rope segments ---
    SDL_SetRenderDrawColor(renderer, 180, 140, 80, 255); // brownish rope color

    for (int i = 0; i < ROPE_SEGMENTS; i++)
    {
        const RopeNode *a = &r->nodes[i];
        const RopeNode *b = &r->nodes[i + 1];
        SDL_RenderLine(renderer, a->x, a->y, b->x, b->y);
    }

    // --- Draw bucket at the last node ---
    const RopeNode *tip = &r->nodes[ROPE_SEGMENTS];
    SDL_FRect bucket = {
        tip->x - BUCKET_W * 0.5f,
        tip->y,
        BUCKET_W,
        BUCKET_H};
    SDL_SetRenderDrawColor(renderer, 100, 100, 110, 255); // dark metal gray
    SDL_RenderFillRect(renderer, &bucket);

    // Bucket rim (slightly wider highlight line)
    SDL_SetRenderDrawColor(renderer, 150, 150, 160, 255);
    SDL_FRect rim = {bucket.x - 1.0f, bucket.y, BUCKET_W + 2.0f, 2.0f};
    SDL_RenderFillRect(renderer, &rim);
}
