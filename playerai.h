#ifndef PLAYERAI_H
#define PLAYERAI_H

#include <c64/types.h>

void playerai_select_move(byte team);

void playerai_select_battles(byte team);

#pragma compile("playerai.c")


#endif
