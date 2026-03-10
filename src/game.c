#include "game.h"

#include "renderer.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "tilemap.h"
#include "assets_embedded.h"
#include <stdlib.h>

void game_init(Game *g, SDL_Renderer *r)
{
    g->renderer = r;
    g->scroll = 0;
    g->shootCooldown = 0;
    g->spawnTimer = 0;
    g->score = 0;
    g->elapsed = 0.0f;
    g->game_over = false;

    srand((unsigned int)SDL_GetTicks());

    tilemap_load_mem(&g->map, level_txt, level_txt_len);

    player_init(&g->player, 100, 200);

    bullets_init();
    enemies_init();

    for (int i = 0; i < 5; i++)
        enemies_spawn(600.0f + i * 200.0f, 150.0f + i * 40.0f);
}

void game_update(Game *g, float dt)
{
    // restart on R when game over
    if (g->game_over)
    {
        const bool *keys = SDL_GetKeyboardState(NULL);
        if (keys && keys[SDL_SCANCODE_R])
            game_init(g, g->renderer);
        return;
    }

    g->scroll += 120 * dt;
    g->elapsed += dt;

    player_update(&g->player, dt);
    bullets_update(dt);
    enemies_update(dt);

    //----------------------------------------
    // player shooting
    //----------------------------------------

    const bool *keys = SDL_GetKeyboardState(NULL);
    g->shootCooldown -= dt;

    if (keys && keys[SDL_SCANCODE_SPACE] && g->shootCooldown <= 0)
    {
        bullets_spawn(
            g->player.x + g->player.w,
            g->player.y + g->player.h / 2.0f);

        g->shootCooldown = SHOOT_COOLDOWN;
    }

    //----------------------------------------
    // bullet <-> enemy collisions
    //----------------------------------------

    int killed = enemies_check_bullet_collisions(g->scroll);
    g->score += killed * 10;

    //----------------------------------------
    // enemy <-> player collisions
    //----------------------------------------

    if (enemies_collide_rect(g->player.x, g->player.y,
                             g->player.w, g->player.h,
                             g->scroll))
    {
        player_take_hit(&g->player);
    }

    if (!player_is_alive(&g->player))
        g->game_over = true;

    //----------------------------------------
    // spawn new enemies over time
    //----------------------------------------

    g->spawnTimer += dt;
    float spawnInterval = SPAWN_INTERVAL - g->elapsed * 0.02f;
    if (spawnInterval < 0.5f)
        spawnInterval = 0.5f;
    if (g->spawnTimer >= spawnInterval)
    {
        g->spawnTimer = 0;
        enemies_spawn(
            g->scroll + SCREEN_W + 100.0f,
            50.0f + (float)(rand() % (SCREEN_H - 100)));
    }
}

void game_render(Game *g)
{
    renderer_begin(g->renderer);

    tilemap_render(&g->map, g->renderer, g->scroll);
    player_render(&g->player, g->renderer);
    bullets_render(g->renderer);
    enemies_render(g->renderer, g->scroll);

    //----------------------------------------
    // HUD: health as yellow squares
    //----------------------------------------

    for (int i = 0; i < g->player.hp; i++)
    {
        SDL_FRect hp = {10.0f + i * 20.0f, 10.0f, 15.0f, 15.0f};
        SDL_SetRenderDrawColor(g->renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(g->renderer, &hp);
    }

    char score_text[32];
    SDL_snprintf(score_text, sizeof(score_text), "SCORE: %d", g->score);
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(g->renderer, 10.0f, 35.0f, score_text);

    //----------------------------------------
    // game over – red overlay
    //----------------------------------------

    if (g->game_over)
    {
        SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g->renderer, 200, 0, 0, 160);
        SDL_FRect overlay = {0, 0, SCREEN_W, SCREEN_H};
        SDL_RenderFillRect(g->renderer, &overlay);
        SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_NONE);

        SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(g->renderer, 364.0f, 270.0f, "GAME OVER");
        SDL_RenderDebugText(g->renderer, 328.0f, 290.0f, "Press R to restart");
    }

    renderer_end(g->renderer);
}

void game_destroy(Game *g)
{
    (void)g;
}
