#include "renderer.h"

void renderer_begin(SDL_Renderer *r)
{
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
}

void renderer_end(SDL_Renderer *r)
{
    SDL_RenderPresent(r);
}
