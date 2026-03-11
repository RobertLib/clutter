#pragma once

#include "constants.h"
#include "player.h"

typedef struct
{
    float scroll;  // world-space X offset
    float level_w; // total width of the level in pixels
} Camera;

void camera_init(Camera *c, float level_w);
void camera_update(Camera *c, Player *p, float dt);
