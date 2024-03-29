#ifndef STATUS_H
#define STATUS_H

#include "units.h"
#include "hexdisplay.h"

// 0----+----1----+----2----+----3----+----
// XX YY MOUNTAIN PLR 1 3x 1ST INFANTRY    

void status_init(void);

void status_update_pos(char x, char y);

void status_update_state(const char * state, byte color);

void status_update_unit(char unit);

void status_show_progress(const char * action, byte progress);


#pragma compile("status.c")

#endif
