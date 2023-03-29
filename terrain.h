#ifndef TERRAIN_H
#define TERRAIN_H

#include <c64/types.h>

// Build terrain from fixed seed

void terrain_build(unsigned seed);

// Patch a line in the terrain, for e.g. a road

void terrain_patch(char px, char py, char dir, char len, char type);

// Patch a circle in the terrain, for e.g. a plateau

void terrain_patch_circle(char px, char py, char rad, char type);

#pragma compile("terrain.c")

#endif
