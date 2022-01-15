#ifndef HEXMAP_H
#define HEXMAP_H

#include <c64/types.h>

extern const unsigned hex_x_sq[32];
extern const unsigned hex_y_sq[64];

// return squared distance of a and b times 32

unsigned hex_dist_square(byte xa, byte ya, byte xb, byte yb);

inline unsigned hex_size_square(byte sz);

#pragma compile("hexmap.c")

#endif
