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
#include "gamemusic.h"
#include "mainmenu.h"
#include <audio/sidfx.h>

int main(void)
{
	// Turn all interrupts off
	
	cia_init();

	__asm { sei }

	// Turn display off

	vic.ctrl1 = VIC_CTRL1_BMM | VIC_CTRL1_ECM;

	// Turn basic ROM off
	
	mmap_set(MMAP_NO_ROM);

	initDisplay();

	grid_blank();

	drawBaseGrid();
	status_init();

	split_init();

	sid.fmodevol = 0x0f;

	game_init();

	// Main loop, triggers at max once each frame

	for(;;)
	{
		while (vic.ctrl1 & VIC_CTRL1_RST8) ;
		while (!(vic.ctrl1 & VIC_CTRL1_RST8)) ;

		game_loop();
	}

	return 0;
}
