#pragma once

#include <SDL3/SDL.h>
#include "tilemap.h"

// Number of rope segments (one more node than segments)
#define ROPE_SEGMENTS 12
#define ROPE_SEG_LEN 7.0f // rest length of each segment in pixels
#define ROPE_ITER 8       // constraint relaxation iterations per step

// Bucket size
#define BUCKET_W 14.0f
#define BUCKET_H 12.0f

typedef struct
{
    float x, y;   // current position
    float px, py; // previous position (Verlet)
} RopeNode;

typedef struct
{
    RopeNode nodes[ROPE_SEGMENTS + 1]; // nodes[0] = anchor at plane bottom

    float water_level;   // 0.0 = empty, 1.0 = full
    float prev_anchor_x; // previous anchor position for velocity computation (world-space)
    float prev_anchor_y;
    float spill_emit_acc; // time accumulator for spill particle emissions
} Rope;

void rope_init(Rope *r, float anchor_x, float anchor_y);
// map + scroll are needed for water detection and particle emission
void rope_update(Rope *r, float anchor_x, float anchor_y, float dt,
                 const Tilemap *map, float scroll);
void rope_render(const Rope *r, SDL_Renderer *renderer, float scroll);
