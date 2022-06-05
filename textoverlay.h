#ifndef TEXTOVERLAY_H
#define TEXTOVERLAY_H


void tovl_show(const char * text, char color);

void tovl_hide(void);

void tovl_wait(void);

void tovl_color(char line, char color);

#pragma compile("textoverlay.c")


#endif

