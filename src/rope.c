#include "rope.h"
#include "particle.h"
#include <math.h>
#include <stdlib.h>

#define ROPE_GRAVITY 600.0f // downward acceleration (px/s²)
#define ROPE_DAMPING 0.98f  // velocity damping to avoid endless oscillation

// Bucket water mechanics
#define BUCKET_FILL_RATE 0.45f       // water_level/s while submerged in water
#define BUCKET_SPILL_THRESHOLD 90.0f // px/s anchor speed above which water spills
#define BUCKET_SPILL_RATE 0.55f      // water_level/s drained when above threshold
#define BUCKET_SPILL_EMIT_INT 0.045f // seconds between spray particle emissions

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
    r->water_level = 0.0f;
    r->prev_anchor_x = anchor_x;
    r->prev_anchor_y = anchor_y;
    r->spill_emit_acc = 0.0f;
}

void rope_update(Rope *r, float anchor_x, float anchor_y, float dt,
                 const Tilemap *map, float scroll)
{
    float dt2 = dt * dt;

    // --- Anchor velocity (screen-space) ---
    float avx = 0.0f, avy = 0.0f;
    if (dt > 0.0001f)
    {
        avx = (anchor_x - r->prev_anchor_x) / dt;
        avy = (anchor_y - r->prev_anchor_y) / dt;
    }
    r->prev_anchor_x = anchor_x;
    r->prev_anchor_y = anchor_y;

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
                b->x -= dx * diff;
                b->y -= dy * diff;
            }
            else
            {
                b->x -= dx * diff * 0.5f;
                b->y -= dy * diff * 0.5f;
                a->x += dx * diff * 0.5f;
                a->y += dy * diff * 0.5f;
            }
        }
    }

    // --- 4. Water detection and bucket filling ---
    const RopeNode *tip = &r->nodes[ROPE_SEGMENTS];
    float bucket_wx = tip->x + scroll;
    float bucket_wy = tip->y;

    int tile = tilemap_overlaps_type(map,
                                     bucket_wx - BUCKET_W * 0.5f,
                                     bucket_wy,
                                     BUCKET_W, BUCKET_H);
    if (tile == TILE_WATER)
    {
        r->water_level += BUCKET_FILL_RATE * dt;
        if (r->water_level > 1.0f)
            r->water_level = 1.0f;
    }

    // --- 5. Water spilling on fast movement ---
    // Horizontal component dominates (centrifugal effect when turning)
    float anchor_speed = fabsf(avx) * 0.75f + fabsf(avy) * 0.25f;

    if (anchor_speed > BUCKET_SPILL_THRESHOLD && r->water_level > 0.0f)
    {
        float excess = (anchor_speed - BUCKET_SPILL_THRESHOLD) / BUCKET_SPILL_THRESHOLD;
        float spill_dt = BUCKET_SPILL_RATE * (1.0f + excess) * dt;
        r->water_level -= spill_dt;
        if (r->water_level < 0.0f)
            r->water_level = 0.0f;

        // Emit water spray particles from the bucket tip (world-space)
        r->spill_emit_acc += dt;
        while (r->spill_emit_acc >= BUCKET_SPILL_EMIT_INT)
        {
            r->spill_emit_acc -= BUCKET_SPILL_EMIT_INT;
            // Spray slightly randomised around the bucket tip
            float ex = bucket_wx + ((float)rand() / RAND_MAX - 0.5f) * BUCKET_W;
            float ey = bucket_wy + ((float)rand() / RAND_MAX) * (BUCKET_H * 0.4f);
            particles_emit(ex, ey, PARTICLE_WATER, 30, 120, 220);
        }
    }
    else
    {
        r->spill_emit_acc = 0.0f;
    }
}

void rope_render(const Rope *r, SDL_Renderer *renderer)
{
    // --- Draw rope segments ---
    SDL_SetRenderDrawColor(renderer, 180, 140, 80, 255);

    for (int i = 0; i < ROPE_SEGMENTS; i++)
    {
        const RopeNode *a = &r->nodes[i];
        const RopeNode *b = &r->nodes[i + 1];
        SDL_RenderLine(renderer, a->x, a->y, b->x, b->y);
    }

    // --- Compute bucket orientation from the last rope segment ---
    const RopeNode *tip = &r->nodes[ROPE_SEGMENTS];
    const RopeNode *pre = &r->nodes[ROPE_SEGMENTS - 1];

    float dx = tip->x - pre->x;
    float dy = tip->y - pre->y;
    float len = sqrtf(dx * dx + dy * dy);

    // Bucket axis (along the last segment, i.e. "downward")
    float ax = 0.0f, ay = 1.0f;
    if (len > 0.001f)
    {
        ax = dx / len;
        ay = dy / len;
    }
    // Perpendicular (for bucket width)
    float px_dir = -ay;
    float py_dir = ax;

    float hw = BUCKET_W * 0.5f;

    // Four corners of the bucket
    float rim_lx = tip->x - px_dir * hw;
    float rim_ly = tip->y - py_dir * hw;
    float rim_rx = tip->x + px_dir * hw;
    float rim_ry = tip->y + py_dir * hw;

    float bot_lx = rim_lx + ax * BUCKET_H;
    float bot_ly = rim_ly + ay * BUCKET_H;
    float bot_rx = rim_rx + ax * BUCKET_H;
    float bot_ry = rim_ry + ay * BUCKET_H;

    // --- Bucket body (grey metal, subtle gradient – sides darker) ---
    SDL_Vertex bucket_verts[4] = {
        {{rim_lx, rim_ly}, {100 / 255.0f, 100 / 255.0f, 115 / 255.0f, 1.0f}, {0, 0}},
        {{rim_rx, rim_ry}, {100 / 255.0f, 100 / 255.0f, 115 / 255.0f, 1.0f}, {0, 0}},
        {{bot_rx, bot_ry}, {75 / 255.0f, 75 / 255.0f, 85 / 255.0f, 1.0f}, {0, 0}},
        {{bot_lx, bot_ly}, {75 / 255.0f, 75 / 255.0f, 85 / 255.0f, 1.0f}, {0, 0}},
    };
    int bucket_idx[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer, NULL, bucket_verts, 4, bucket_idx, 6);

    // --- Water inside the bucket ---
    if (r->water_level > 0.001f)
    {
        // Water surface: measured from the bottom toward the rim
        float fill_top = 1.0f - r->water_level; // 0 = full, 1 = empty

        float wt_lx = rim_lx + ax * (BUCKET_H * fill_top);
        float wt_ly = rim_ly + ay * (BUCKET_H * fill_top);
        float wt_rx = rim_rx + ax * (BUCKET_H * fill_top);
        float wt_ry = rim_ry + ay * (BUCKET_H * fill_top);

        SDL_Vertex water_verts[4] = {
            {{wt_lx, wt_ly}, {60 / 255.0f, 160 / 255.0f, 240 / 255.0f, 0.90f}, {0, 0}},
            {{wt_rx, wt_ry}, {60 / 255.0f, 160 / 255.0f, 240 / 255.0f, 0.90f}, {0, 0}},
            {{bot_rx, bot_ry}, {20 / 255.0f, 90 / 255.0f, 210 / 255.0f, 0.95f}, {0, 0}},
            {{bot_lx, bot_ly}, {20 / 255.0f, 90 / 255.0f, 210 / 255.0f, 0.95f}, {0, 0}},
        };
        int water_idx[6] = {0, 1, 2, 0, 2, 3};

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderGeometry(renderer, NULL, water_verts, 4, water_idx, 6);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    // --- Bucket rim (bright line across full width) ---
    SDL_SetRenderDrawColor(renderer, 160, 160, 170, 255);
    // Widen the rim by 1px on each side
    SDL_RenderLine(renderer,
                   (int)(rim_lx - px_dir), (int)(rim_ly - py_dir),
                   (int)(rim_rx + px_dir), (int)(rim_ry + py_dir));
    SDL_RenderLine(renderer,
                   (int)rim_lx, (int)rim_ly,
                   (int)rim_rx, (int)rim_ry);
}
