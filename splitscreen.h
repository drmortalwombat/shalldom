#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

#include <c64/rasterirq.h>

extern RIRQCode	rirqtop, rirqbottom;
extern RIRQCode	rirqoverlay[3];

extern char sprovlx[3];
extern char sprovlimg[3][8];

void split_init(void);

void split_overlay_show(void);

void split_overlay_hide(void);

#pragma compile("splitscreen.c")

#endif
