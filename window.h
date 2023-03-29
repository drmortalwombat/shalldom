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

char window_write(char x, char y, const char * text);

char window_write_uint(char x, char y, unsigned u);

// Copy sprite image into bitmap of window
void window_put_sprite(char x, char y, const char * sprite);

void window_draw_vbar(char x, char y, char h, char c);

// Draw colored 4x4 quad for minimap
void window_draw_quad(char x, char y, char c);

// Draw unit for minimap
void window_draw_quad_u(char x, char y);

// Clear sprite image from bitmap
void window_clear_sprite(char x, char y, char fill);

#pragma compile("window.c")

#endif
