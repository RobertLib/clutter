#pragma once

#include <SDL3/SDL.h>

#define MAX_MAP_W 256
#define MAX_MAP_H 128
#define TILE_SIZE 32

typedef struct
{
    int width;
    int height;

    int data[MAX_MAP_H][MAX_MAP_W];

} Tilemap;

void tilemap_load(Tilemap *m, const char *file);
void tilemap_load_mem(Tilemap *m, const unsigned char *data, unsigned int len);
void tilemap_render(Tilemap *m, SDL_Renderer *r, float scroll);
void tilemap_emit_fire_particles(Tilemap *m, float scroll, float dt);
