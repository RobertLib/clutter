#include "game.h"

#include "renderer.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "tilemap.h"
#include "particle.h"
#include "assets_embedded.h"
#include "camera.h"
#include <stdlib.h>

void game_init(Game *g, SDL_Renderer *r)
{
    g->renderer = r;
    g->shootCooldown = 0;
    g->spawnTimer = 0;
    g->score = 0;
    g->elapsed = 0.0f;
    g->game_over = false;

    srand((unsigned int)SDL_GetTicks());

    tilemap_load_mem(&g->map, level_txt, level_txt_len);

    float level_w = (float)(g->map.width * TILE_SIZE);
    camera_init(&g->camera, level_w);

    player_init(&g->player,
                (float)SCREEN_W / 2.0f - PLAYER_W / 2.0f,
                (float)SCREEN_H / 2.0f - PLAYER_H / 2.0f);

    bullets_init();
    enemies_init();
    particles_init();

    // initial enemies: spread around the player's starting world position
    float start_wx = g->camera.scroll + SCREEN_W * 0.5f;
    for (int i = 0; i < 5; i++)
    {
        // alternate left / right, ~one screen away
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        float ex = start_wx + side * (SCREEN_W * 0.8f + i * 150.0f);
        float ey = 80.0f + i * 60.0f;
        enemies_spawn(ex, ey, -side * ENEMY_SPEED);
    }
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

    g->elapsed += dt;

    player_update(&g->player, dt);

    camera_update(&g->camera, &g->player, dt);
    bullets_update(dt);
    enemies_update(dt, g->camera.scroll);
    tilemap_emit_fire_particles(&g->map, g->camera.scroll, dt);
    particles_update(dt);

    //----------------------------------------
    // player shooting
    //----------------------------------------

    const bool *keys = SDL_GetKeyboardState(NULL);
    g->shootCooldown -= dt;

    if (keys && keys[SDL_SCANCODE_SPACE] && g->shootCooldown <= 0)
    {
        float bvx = BULLET_SPEED * g->player.facing;
        float bx = (g->player.facing > 0)
                       ? g->player.x + g->player.w // shoot from right edge
                       : g->player.x - BULLET_W;   // shoot from left edge
        bullets_spawn(bx,
                      g->player.y + g->player.h / 2.0f - BULLET_H / 2.0f,
                      bvx);

        g->shootCooldown = SHOOT_COOLDOWN;
    }

    //----------------------------------------
    // bullet <-> enemy collisions
    //----------------------------------------

    int killed = enemies_check_bullet_collisions(g->camera.scroll);
    g->score += killed * 10;

    //----------------------------------------
    // enemy <-> player collisions
    //----------------------------------------

    if (enemies_collide_rect(g->player.x, g->player.y,
                             g->player.w, g->player.h,
                             g->camera.scroll))
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
        // randomly spawn from left or right side of the screen
        float side = (rand() % 2 == 0) ? 1.0f : -1.0f;
        float ex = g->camera.scroll + SCREEN_W * 0.5f + side * (SCREEN_W * 0.5f + 120.0f);
        float ey = 50.0f + (float)(rand() % (SCREEN_H - 100));
        enemies_spawn(ex, ey, -side * ENEMY_SPEED);
    }
}

void game_render(Game *g)
{
    renderer_begin(g->renderer);

    tilemap_render(&g->map, g->renderer, g->camera.scroll);
    particles_render(g->renderer, g->camera.scroll);
    player_render(&g->player, g->renderer);
    bullets_render(g->renderer);
    enemies_render(g->renderer, g->camera.scroll);

    //----------------------------------------
    // HUD: score left, health right
    //----------------------------------------

    char score_text[32];
    SDL_snprintf(score_text, sizeof(score_text), "SCORE: %d", g->score);
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    SDL_SetRenderScale(g->renderer, 2.0f, 2.0f);
    SDL_RenderDebugText(g->renderer, 10.0f / 2.0f, 10.0f / 2.0f, score_text);
    SDL_SetRenderScale(g->renderer, 1.0f, 1.0f);

    // health squares anchored to the right edge
    for (int i = 0; i < g->player.hp; i++)
    {
        float hx = (float)SCREEN_W - 10.0f - (g->player.hp - i) * 20.0f;
        SDL_FRect hp = {hx, 10.0f, 16.0f, 16.0f};
        SDL_SetRenderDrawColor(g->renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(g->renderer, &hp);
    }

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
        SDL_SetRenderScale(g->renderer, 2.0f, 2.0f);
        SDL_RenderDebugText(g->renderer, 164.0f, 135.0f, "GAME OVER");
        SDL_RenderDebugText(g->renderer, 128.0f, 155.0f, "Press R to restart");
        SDL_SetRenderScale(g->renderer, 1.0f, 1.0f);
    }

    renderer_end(g->renderer);
}

void game_destroy(Game *g)
{
    (void)g;
}
