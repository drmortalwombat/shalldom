#ifndef CURSOR_H
#define CURSOR_H

#include <c64/types.h>

extern int cursorX, cursorY;
extern char gridX, gridY;



void cursor_init(char cx, char cy);

void cursor_move(sbyte dx, sbyte dy);

void cursor_show(void);

void cursor_hide(void);


#pragma compile("cursor.c")

#endif
