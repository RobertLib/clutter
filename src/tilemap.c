#include "tilemap.h"
#include "constants.h"
#include <stdio.h>
#include <string.h>

void tilemap_load(Tilemap *m, const char *file)
{
    FILE *f = fopen(file, "r");
    if (!f)
    {
        SDL_Log("tilemap_load: cannot open '%s'", file);
        return;
    }

    m->width = 0;
    m->height = 0;

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        if (m->height >= MAX_MAP_H)
            break;

        int len = (int)strlen(line);

        // strip trailing newline / carriage-return
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            len--;

        // clamp to array width
        if (len > MAX_MAP_W)
            len = MAX_MAP_W;

        for (int x = 0; x < len; x++)
            m->data[m->height][x] = (line[x] == '1') ? 1 : 0;

        if (len > m->width)
            m->width = len;

        m->height++;
    }

    fclose(f);
}

void tilemap_load_mem(Tilemap *m, const unsigned char *data, unsigned int len)
{
    m->width = 0;
    m->height = 0;

    unsigned int pos = 0;
    while (pos < len && m->height < MAX_MAP_H)
    {
        unsigned int start = pos;
        while (pos < len && data[pos] != '\n')
            pos++;

        int line_len = (int)(pos - start);
        if (line_len > 0 && data[start + line_len - 1] == '\r')
            line_len--;
        if (line_len > MAX_MAP_W)
            line_len = MAX_MAP_W;

        for (int x = 0; x < line_len; x++)
            m->data[m->height][x] = (data[start + x] == '1') ? 1 : 0;

        if (line_len > m->width)
            m->width = line_len;

        m->height++;
        if (pos < len)
            pos++; /* skip '\n' */
    }
}

void tilemap_render(Tilemap *m, SDL_Renderer *r, float scroll)
{
    int screenTilesX = SCREEN_W / TILE_SIZE + 2;
    int screenTilesY = SCREEN_H / TILE_SIZE + 2;
    int startTileX = (int)(scroll / TILE_SIZE);

    SDL_SetRenderDrawColor(r, 100, 200, 100, 255);

    for (int y = 0; y < screenTilesY && y < m->height; y++)
    {
        for (int x = 0; x < screenTilesX; x++)
        {
            int mapX = startTileX + x;

            if (mapX < 0 || mapX >= m->width)
                continue;

            if (m->data[y][mapX] == 0)
                continue;

            SDL_FRect rect = {
                mapX * TILE_SIZE - scroll,
                (float)(y * TILE_SIZE),
                TILE_SIZE,
                TILE_SIZE};
            SDL_RenderFillRect(r, &rect);
        }
    }
}
