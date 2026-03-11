#include "tilemap.h"
#include "particle.h"
#include "constants.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
        {
            if (line[x] == '1')
                m->data[m->height][x] = TILE_GROUND;
            else if (line[x] == '2')
                m->data[m->height][x] = TILE_WATER;
            else
                m->data[m->height][x] = 0;
        }

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
        {
            char ch = (char)data[start + x];
            if (ch == '1')
                m->data[m->height][x] = TILE_GROUND;
            else if (ch == '2')
                m->data[m->height][x] = TILE_WATER;
            else
                m->data[m->height][x] = 0;
        }

        if (line_len > m->width)
            m->width = line_len;

        m->height++;
        if (pos < len)
            pos++; /* skip '\n' */
    }
}

void tilemap_render(Tilemap *m, SDL_Renderer *r, float scroll)
{
    int screen_tiles_x = SCREEN_W / TILE_SIZE + 2;
    int screen_tiles_y = SCREEN_H / TILE_SIZE + 2;
    int start_tile_x = (int)(scroll / TILE_SIZE);

    for (int y = 0; y < screen_tiles_y && y < m->height; y++)
    {
        for (int x = 0; x < screen_tiles_x; x++)
        {
            int mapX = start_tile_x + x;

            if (mapX < 0 || mapX >= m->width)
                continue;

            int tile = m->data[y][mapX];
            if (tile == 0)
                continue;

            if (tile == TILE_WATER)
                SDL_SetRenderDrawColor(r, 30, 120, 220, 255);
            else
                SDL_SetRenderDrawColor(r, 100, 200, 100, 255);

            SDL_FRect rect = {
                mapX * TILE_SIZE - scroll,
                (float)(y * TILE_SIZE),
                TILE_SIZE,
                TILE_SIZE};
            SDL_RenderFillRect(r, &rect);
        }
    }
}

void tilemap_emit_fire_particles(Tilemap *m, float scroll, float dt)
{
    /* Particles per second emitted per visible top tile */
    static const float RATE = 10.0f;

    int startX = (int)(scroll / TILE_SIZE);
    if (startX < 0)
        startX = 0;
    int endX = startX + SCREEN_W / TILE_SIZE + 2;
    if (endX >= m->width)
        endX = m->width - 1;

    for (int x = startX; x <= endX; x++)
    {
        /* Scan top-to-bottom; first filled tile in column is the top surface */
        for (int y = 0; y < m->height; y++)
        {
            if (m->data[y][x] == 0)
                continue;

            /* Only ground tiles emit fire */
            if (m->data[y][x] != TILE_GROUND)
                break;

            /* Emit RATE particles/sec from the top edge of this tile */
            float expected = RATE * dt;
            int count = (int)expected;
            if ((float)rand() / (float)RAND_MAX < (expected - (float)count))
                count++;

            for (int n = 0; n < count; n++)
            {
                float wx = (float)(x * TILE_SIZE) +
                           (float)(rand() % TILE_SIZE);
                float wy = (float)(y * TILE_SIZE);
                particles_emit(wx, wy, PARTICLE_FIRE, 0, 0, 0);
            }
            break; /* Only the topmost tile per column */
        }
    }
}

bool tilemap_overlaps_rect(const Tilemap *m, float wx, float wy, float rw, float rh)
{
    int col0 = (int)(wx / TILE_SIZE);
    int col1 = (int)((wx + rw - 1.0f) / TILE_SIZE);
    int row0 = (int)(wy / TILE_SIZE);
    int row1 = (int)((wy + rh - 1.0f) / TILE_SIZE);

    if (col0 < 0)
        col0 = 0;
    if (row0 < 0)
        row0 = 0;
    if (col1 >= m->width)
        col1 = m->width - 1;
    if (row1 >= m->height)
        row1 = m->height - 1;

    for (int row = row0; row <= row1; row++)
        for (int col = col0; col <= col1; col++)
            if (m->data[row][col] != 0)
                return true;

    return false;
}

int tilemap_overlaps_type(const Tilemap *m, float wx, float wy, float rw, float rh)
{
    int col0 = (int)(wx / TILE_SIZE);
    int col1 = (int)((wx + rw - 1.0f) / TILE_SIZE);
    int row0 = (int)(wy / TILE_SIZE);
    int row1 = (int)((wy + rh - 1.0f) / TILE_SIZE);

    if (col0 < 0)
        col0 = 0;
    if (row0 < 0)
        row0 = 0;
    if (col1 >= m->width)
        col1 = m->width - 1;
    if (row1 >= m->height)
        row1 = m->height - 1;

    for (int row = row0; row <= row1; row++)
        for (int col = col0; col <= col1; col++)
            if (m->data[row][col] != 0)
                return m->data[row][col];

    return 0;
}

void tilemap_emit_water_particles(Tilemap *m, float scroll, float dt)
{
    static const float RATE = 0.8f;

    int startX = (int)(scroll / TILE_SIZE);
    if (startX < 0)
        startX = 0;
    int endX = startX + SCREEN_W / TILE_SIZE + 2;
    if (endX >= m->width)
        endX = m->width - 1;

    for (int x = startX; x <= endX; x++)
    {
        for (int y = 0; y < m->height; y++)
        {
            if (m->data[y][x] == 0)
                continue;

            if (m->data[y][x] != TILE_WATER)
                break;

            /* Emit splash particles from the top edge of the water tile */
            float expected = RATE * dt;
            int count = (int)expected;
            if ((float)rand() / (float)RAND_MAX < (expected - (float)count))
                count++;

            for (int n = 0; n < count; n++)
            {
                float wx = (float)(x * TILE_SIZE) +
                           (float)(rand() % TILE_SIZE);
                float wy = (float)(y * TILE_SIZE);
                particles_emit(wx, wy, PARTICLE_WATER, 0, 0, 0);
            }
            break;
        }
    }
}
