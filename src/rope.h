#pragma once

#include <SDL3/SDL.h>

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
} Rope;

void rope_init(Rope *r, float anchor_x, float anchor_y);
void rope_update(Rope *r, float anchor_x, float anchor_y, float dt);
void rope_render(const Rope *r, SDL_Renderer *renderer);
