#include <SDL3/SDL.h>
#include <stdio.h>
#include "game.h"

#define TARGET_FPS 60
#define FRAME_MS (1000 / TARGET_FPS)

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Clutter", SCREEN_W, SCREEN_H, 0);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    Game game;
    game_init(&game, renderer);

    bool running = true;
    Uint64 last = SDL_GetTicks();

    while (running)
    {
        Uint64 frame_start = SDL_GetTicks();

        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }

        Uint64 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        if (dt > 0.1f)
            dt = 0.1f;
        last = now;

        game_update(&game, dt);
        game_render(&game);

        // Uint64 frame_time = SDL_GetTicks() - frame_start;
        // if (frame_time < FRAME_MS)
        //     SDL_Delay(FRAME_MS - frame_time);
    }

    game_destroy(&game);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
