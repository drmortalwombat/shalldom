#ifndef TERRAIN_H
#define TERRAIN_H

#include <c64/types.h>

void terrain_build(unsigned seed);

void terrain_pave_road(char px, char py, sbyte dx, sbyte dy, byte s);

#pragma compile("terrain.c")

#endif
