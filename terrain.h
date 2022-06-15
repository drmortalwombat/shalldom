#ifndef TERRAIN_H
#define TERRAIN_H

#include <c64/types.h>

void terrain_build(unsigned seed);

void terrain_patch(char px, char py, char dir, char len, char type);

void terrain_patch_circle(char px, char py, char rad, char type);

#pragma compile("terrain.c")

#endif
