#include "mainmenu.h"

#include "perlin.h"
#include "hexdisplay.h"
#include <c64/vic.h>
#include "textoverlay.h"
#include "gamemusic.h"
#include <stdlib.h>

#pragma section( mainmenu, 0, , , bss)

#pragma region( mainmenu, 0xc000, 0xc800, , , {mainmenu} )

#pragma bss( mainmenu )

signed char	mgrid[2][9][13];
char		cammocolors[256];

#pragma bss(bss)

static const char cammocenter[8] = {
	VCOL_BLUE,
	VCOL_DARK_GREY,
	VCOL_ORANGE,
	VCOL_MED_GREY,

	VCOL_GREEN,
	VCOL_LT_GREY,
	VCOL_YELLOW,
	VCOL_WHITE,
};

static const char * mainmenu_titles[4] = {
		"SHALLOW\n"
		" DOMAINS\n"
		"  A C64\n"
		"STRATEGY\n"
		"  GAME",

		"CODE\n"
		"DRMORTAL\n"
		"  WOMBAT\n" 
		"MUSIC\n"
		"  CRISPS\n",

		"TESTERS\n"
		"TBD...",

		"VISIT US\n"
		"ON\n"
		"ITCH.IO\n"
		"FOR MORE"
};

void mainmenu_open(void)
{
	music_patch_voice3(true);

	music_init(TUNE_MAIN_MENU);

	for(int y=0; y<9; y++)
		for(int x=0; x<13; x++)
			mgrid[0][y][x] = 32;

	for(char i=0; i<128; i++)
	{
		cammocolors[i] = VCOL_BLACK;
		cammocolors[i+128] = VCOL_WHITE;
	}

	for(char i=0; i<16; i++)
		cammocolors[120 + i] = cammocenter[i >> 1];		

	char	gy, gx, tg, coff, doff, state;
	bool	first = true;

	tg = 0;
	coff = 96;
	state = 0;

	while(true)
	{
		mnoise_init(rand());

		gy = 1;
		gx = 0;
		doff = 1;

		do  
		{
			switch (state & 0x3f)
			{
			case 0:
				tovl_show(mainmenu_titles[state >> 6], VCOL_YELLOW);
				break;
			case 0x30:
				tovl_hide();
				break;
			}

			tovl_check();

			if (gy < 9)
			{
				mgrid[1-tg][gy][gx] = mnoise2(gx * 32, (2 * gy + (gx & 1)) * 16, 3, 103) >> 9;

				gx++;
				if (gx == 13)
				{
					gx = 0;
					gy++;
				}
			}

			if (first)
				state = 1;
			else
			{
				state++;

				for(char y=1; y<9; y++)
				{
					for(char x=0; x<13; x++)
					{
						char f = mgrid[tg][y][x] + coff;

						viewcolor[y][x] = cammocolors[f];
					}
				}

				grid_redraw_colors();
			}

			coff += doff;
			if (coff == 160)
				doff = -1;

			joy_poll(0);
			keyb_poll();

			if (joyb[0] || keyb_codes[keyb_key & 0x7f] == ' ')
			{
				tovl_hide();
				tovl_wait();

				return;				
			}

		} while (coff != 96);

		first = false;
		tg = 1 - tg;
	}
}
