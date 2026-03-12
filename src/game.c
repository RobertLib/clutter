#include "game.h"

#include "renderer.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "tilemap.h"
#include "particle.h"
#include "assets_embedded.h"
#include "camera.h"
#include "rope.h"
#include <stdlib.h>

void game_init(Game *g, SDL_Renderer *r)
{
    g->renderer = r;
    timer_clear(&g->shoot_cooldown);
    timer_set(&g->spawn_timer, SPAWN_INTERVAL);
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

    float rope_ax = g->player.x + g->camera.scroll + PLAYER_W * 0.5f; // world-space
    float rope_ay = g->player.y + PLAYER_H;
    rope_init(&g->rope, rope_ax, rope_ay);

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

    //----------------------------------------
    // dying timer – terrain death animation
    //----------------------------------------

    if (g->player.is_dying)
    {
        timer_tick(&g->player.dying_timer, dt);
        if (timer_expired(&g->player.dying_timer))
        {
            if (g->player.hp <= 0)
            {
                g->game_over = true;
                return;
            }
            // Respawn at the starting position
            camera_init(&g->camera, g->camera.level_w);
            g->player.x = (float)SCREEN_W / 2.0f - PLAYER_W / 2.0f;
            g->player.y = (float)SCREEN_H / 2.0f - PLAYER_H / 2.0f;
            g->player.vx = PLANE_FORWARD_SPEED;
            g->player.vy = 0.0f;
            g->player.angle = 0.0f;
            g->player.facing = 1;
            g->player.is_dying = false;
            timer_clear(&g->player.dying_timer);
            timer_set(&g->player.hit_cooldown, 2.0f);
            // Reset rope so nodes don't inherit velocity from the teleport
            rope_init(&g->rope,
                      g->player.x + g->player.w * 0.5f + g->camera.scroll,
                      g->player.y + g->player.h);
        }
    }

    //----------------------------------------
    // terrain collision
    //----------------------------------------

    if (!g->player.is_dying)
    {
        float wx = g->player.x + g->camera.scroll;
        float wy = g->player.y;
        int hit = tilemap_overlaps_type(&g->map, wx, wy, g->player.w, g->player.h);
        if (hit != 0)
        {
            float cx = wx + g->player.w * 0.5f;
            float cy = wy + g->player.h * 0.5f;
            if (hit == TILE_WATER)
                particles_emit(cx, cy, PARTICLE_EXPLOSION, 30, 120, 220);
            else
                particles_emit(cx, cy, PARTICLE_EXPLOSION, 255, 255, 0);
            player_terrain_death(&g->player);
        }
    }

    camera_update(&g->camera, &g->player, dt);

    // Update rope – anchor at the bottom-center of the plane (world-space)
    if (!g->player.is_dying)
    {
        float ax = g->player.x + g->player.w * 0.5f + g->camera.scroll;
        float ay = g->player.y + g->player.h;
        rope_update(&g->rope, ax, ay, dt, &g->map, g->camera.scroll);
    }

    bullets_update(dt);
    enemies_update(dt, g->camera.scroll);
    tilemap_emit_fire_particles(&g->map, g->camera.scroll, dt);
    tilemap_emit_water_particles(&g->map, g->camera.scroll, dt);
    particles_update(dt);

    //----------------------------------------
    // player shooting
    //----------------------------------------

    const bool *keys = SDL_GetKeyboardState(NULL);
    timer_tick(&g->shoot_cooldown, dt);

    if (!g->player.is_dying && keys && keys[SDL_SCANCODE_SPACE] && timer_expired(&g->shoot_cooldown))
    {
        float bvx = BULLET_SPEED * g->player.facing;
        float bx = (g->player.facing > 0)
                       ? g->player.x + g->player.w // shoot from right edge
                       : g->player.x - BULLET_W;   // shoot from left edge
        bullets_spawn(bx,
                      g->player.y + g->player.h / 2.0f - BULLET_H / 2.0f,
                      bvx);

        timer_set(&g->shoot_cooldown, SHOOT_COOLDOWN);
    }

    //----------------------------------------
    // bullet <-> enemy collisions
    //----------------------------------------

    int killed = enemies_check_bullet_collisions(g->camera.scroll);
    g->score += killed * 10;

    //----------------------------------------
    // enemy <-> player collisions
    //----------------------------------------

    if (!g->player.is_dying &&
        enemies_collide_rect(g->player.x, g->player.y,
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

    timer_tick(&g->spawn_timer, dt);
    if (timer_expired(&g->spawn_timer))
    {
        float spawn_interval = SPAWN_INTERVAL - g->elapsed * 0.02f;
        if (spawn_interval < 0.5f)
            spawn_interval = 0.5f;
        timer_set(&g->spawn_timer, spawn_interval);
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
    if (!g->player.is_dying)
        rope_render(&g->rope, g->renderer, g->camera.scroll);
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
