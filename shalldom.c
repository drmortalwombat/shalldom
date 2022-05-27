#include <c64/vic.h>
#include <c64/cia.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <c64/joystick.h>
#include <c64/keyboard.h>
#include <c64/rasterirq.h>
#include <c64/memmap.h>
#include <c64/sid.h>
#include "perlin.h"
#include "hexdisplay.h"
#include "cursor.h"
#include "gameplay.h"
#include "units.h"
#include "status.h"
#include "terrain.h"
#include "playerai.h"
#include "splitscreen.h"
#include "textoverlay.h"
#include "levels.h"
#include <audio/sidfx.h>

#pragma region(main, 0x0a00, 0xa000, , , {code, data, bss, heap, stack} )

#pragma section( music, 0)

#pragma region( music, 0xa000, 0xc000, , , {music} )

#pragma data(music)

__export char music[] = {
	#embed 0x2000 0x88 "ArmyGame.sid" 
}

// 00 - Game theme neutral
// 01 - Results
// 02 - Game theme Enemy winning

#pragma data(data)

void music_init(char tune)
{
	__asm
	{
		lda		tune
		jsr		$a000
	}
}

void music_play(void)
{
	__asm
	{
		jsr		$a003
	}
}

void music_patch_voice3(bool enable)
{
	*(char *)0xa08f = enable ? 0x20 : 0x4c;
}

AITask	aitasks[] = {
	{AIS_IDLE + 0 * 8,  0,  0, 0},
	{AIS_RUSH + 0 * 8,  3, 15, 0},
	{AIS_RUSH + 1 * 8, 17,  9, 0},
	{AIS_RUSH + 1 * 8, 11, 22, 0},
}

int main(void)
{
	cia_init();

	__asm { sei }

	// Turn basic ROM off
	
	mmap_set(MMAP_NO_ROM);

	initDisplay();

	grid_blank();

	drawBaseGrid();
	status_init();

	music_patch_voice3(false);
	
	split_init();

	sid.fmodevol = 0x0f;

	level_setup(0);

#if 0
	tovl_show("PREPARE\nTERRAIN\nDATA\n", VCOL_YELLOW);


	AITasks	=	aitasks;

	terrain_build(23893);
	terrain_pave_road( 4, 16,  1, -1, 13);
	terrain_pave_road(17,  9,  1,  1,  1);
	terrain_pave_road(18, 10,  0,  2,  1);
	terrain_pave_road(18, 11,  1,  1,  3);
	terrain_pave_road(21, 12,  1, -1,  5);
	drawBaseGrid();

	unit_add(UNIT_COMMAND | UNIT_TEAM_1,  3, 15, 0);
	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 29, 16, 0);

	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1,  4, 15, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1,  4, 16, 1);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  5, 14, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  5, 15, 1);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  5, 16, 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  6, 13, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  6, 14, 1);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  7, 13, 2);

	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 18, 11, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 17, 13, 1);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 13, 19, 0 + 8 * 3);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2,  8, 10, 1 + 8 * 1);	
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 22, 14, 2 + 8 * 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 15, 1);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 17, 10, 2);
#endif

#if 0
#if 0
	unit_add(UNIT_CHOPPER | UNIT_TEAM_1, 3, 5, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 3, 3, 0);
#elif 0
	unit_add(UNIT_HOVERCRAFT | UNIT_TEAM_1, 3, 4, 0);
	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 28, 6, 0);
#elif 1
	unit_add(UNIT_AIR_DEFENCE | UNIT_TEAM_1, 3, 5, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 10, 1, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 10, 9, 1);
	unit_add(UNIT_CHOPPER | UNIT_TEAM_2, 11, 6, 0);
	unit_add(UNIT_CHOPPER | UNIT_TEAM_2, 11, 5, 1);
	unit_add(UNIT_BOMBER | UNIT_TEAM_2, 11, 10, 0);
#elif 1
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 10, 1, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1, 9, 2, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 10, 2, 0);
	unit_add(UNIT_ARTILLERY | UNIT_TEAM_1, 3, 5, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 11, 6, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 11, 5, 0);
#elif 1
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 10, 1, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 10, 2, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 9, 2, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2,  9, 1, 0);
#elif 1
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 10, 1, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2,  9, 1, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 10, 2, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 11, 1, 0);

#elif 1
	unit_add(UNIT_ARTILLERY | UNIT_TEAM_1, 4, 4, 0);
	unit_add(UNIT_CHOPPER | UNIT_TEAM_1, 10, 3, 0);

	for(char i=0; i<10; i++)
		unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 5 + 2 * i, 4, i);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 3, 3, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 3, 4, 1);

	unit_add(UNIT_FLAG | UNIT_TEAM_1, 1, 1, 0);
	unit_add(UNIT_FLAG | UNIT_TEAM_2, 10, 1, 0);

#else
	numUnits = 16;
	for(char i=0; i<numUnits; i++)
	{
		units[i].mx = rand() & 31;
		units[i].my = rand() & 31;
		units[i].type = rand() % 6 | ((rand() & 1) ? UNIT_TEAM : 0);
		units[i].id = rand() % 5;
		units[i].count = 5;
	}
#endif
#endif

#if 0
	tovl_hide();
	tovl_wait();

	cursor_init(3, 15);

	drawUnits();
	resetFlags();
	calcVisibility(UNIT_TEAM_1);
	calcThreatened(UNIT_TEAM_2);

	updateColors();
#endif

	game_init();

	for(;;)
	{
		while (vic.ctrl1 & VIC_CTRL1_RST8) ;
		while (!(vic.ctrl1 & VIC_CTRL1_RST8)) ;

		game_loop();
	}

	return 0;
}
