#include "texture.h"

SDL_Texture *texture_load(SDL_Renderer *r, const char *path)
{
    SDL_Surface *s = SDL_LoadBMP(path);

    if (!s)
        return NULL;

    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);

    SDL_DestroySurface(s);

    return t;
}
