#ifndef WINDOW_H
#define WINDOW_H

#include "hexdisplay.h"

void window_open(char x, char y, char w, char h);

void window_close(void);

void window_fill(char pat);

void window_color_rect(char x, char y, char w, char h, char color);

void window_scroll(void);

void window_write(char x, char y, const char * text);

void window_put_sprite(char x, char y, const char * sprite);

#pragma compile("window.c")

#endif
