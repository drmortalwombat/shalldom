#ifndef CURSOR_H
#define CURSOR_H

#include <c64/types.h>

extern int cursorX, cursorY;
extern char gridX, gridY;



void cursor_init(void);

void cursor_move(sbyte dx, sbyte dy);

#pragma compile("cursor.c")

#endif
