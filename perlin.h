#ifndef PERLIN_H
#define PERLIN_H

#include <c64/types.h>

// Initialize noise generator with a 16bit seed
void mnoise_init(unsigned seed);

// Calculate signed 16bit noise value at the given location
// number of octaves and noise factor
int mnoise2(int x, int y, byte noct, int phi);

#pragma compile("perlin.c")

#endif
