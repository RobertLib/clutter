#pragma once

#include <SDL3/SDL.h>
#include "constants.h"
#include "player.h"
#include "tilemap.h"
#include "camera.h"
#include "timer.h"
#include "rope.h"

typedef struct
{
    SDL_Renderer *renderer;

    Player player;
    Tilemap map;
    Camera camera;

    Timer shoot_cooldown;
    Timer spawn_timer;

    Rope rope;

    int score;
    float elapsed;
    bool game_over;

} Game;

void game_init(Game *game, SDL_Renderer *renderer);
void game_update(Game *game, float dt);
void game_render(Game *game);
void game_destroy(Game *game);
