#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

#include <c64/rasterirq.h>

extern RIRQCode	rirqtop, rirqbottom;
extern RIRQCode	rirqoverlay[5];

extern char sprovlc[5];
extern char sprovlx[5];
extern char sprovlimg[5][8];

void split_init(void);

void split_overlay_show(char mask);

void split_overlay_hide(void);

#pragma compile("splitscreen.c")

#endif
