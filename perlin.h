#ifndef PERLIN_H
#define PERLIN_H

#include <c64/types.h>

void mnoise_init(unsigned seed);

int mnoise2(int x, int y, byte noct, int phi);

#pragma compile("perlin.c")

#endif
