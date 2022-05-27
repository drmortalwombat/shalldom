#ifndef TEXTOVERLAY_H
#define TEXTOVERLAY_H


void tovl_show(const char * text, char color);

void tovl_hide(void);

void tovl_wait(void);

#pragma compile("textoverlay.c")


#endif

