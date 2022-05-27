#ifndef WINDOW_H
#define WINDOW_H

#include "hexdisplay.h"

extern char	winX, winY, winW, winH;

void window_open(char x, char y, char w, char h);

void window_close(void);

void window_fill(char pat);

void window_color_rect(char x, char y, char w, char h, char color);

void window_fill_rect(char x, char y, char w, char h, char pat);

void window_scroll(void);

void window_write(char x, char y, const char * text);

void window_put_sprite(char x, char y, const char * sprite);

void window_draw_vbar(char x, char y, char h, char c);

void window_clear_sprite(char x, char y, char fill);

#pragma compile("window.c")

#endif
