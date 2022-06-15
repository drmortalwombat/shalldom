#include "mainmenu.h"

#include "perlin.h"
#include "hexdisplay.h"
#include <c64/vic.h>
#include <c64/keyboard.h>
#include "textoverlay.h"
#include "gamemusic.h"
#include "gameplay.h"
#include "levels.h"
#include <stdlib.h>

#pragma section( mainmenu, 0, , , bss)

#pragma region( mainmenu, 0xc000, 0xc800, , , {mainmenu} )

#pragma bss( mainmenu )

static signed char	mgrid[2][9][13];
static char		cammocolors[256];
static char		mmgx, mmgy, mmtg, mmcoff, mmdoff;
static bool		mmfirst;

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

void mainmenu_anim_init(void)
{
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

	mmtg = 0;
	mmcoff = 96;
	mmfirst = true;

	mmgy = 1;
	mmgx = 0;
	mmdoff = 1;

	mnoise_init(rand());
}

void mainmenu_anim(void)
{
	if (mmgy < 9)
	{
		mgrid[1-mmtg][mmgy][mmgx] = mnoise2(mmgx * 32, (2 * mmgy + (mmgx & 1)) * 16, 3, 103) >> 9;

		mmgx++;
		if (mmgx == 13)
		{
			mmgx = 0;
			mmgy++;
		}
	}

	if (!mmfirst)
	{
		for(char y=1; y<8; y++)
		{
			for(char x=0; x<13; x++)
				viewcolor[y][x] = cammocolors[mgrid[mmtg][y][x] + mmcoff];
		}

		for(char x=0; x<13; x+=2)
			viewcolor[8][x] = cammocolors[mgrid[mmtg][8][x] + mmcoff];

		grid_redraw_colors();
	}

	mmcoff += mmdoff;
	if (mmcoff == 160)
		mmdoff = -1;

	if (mmcoff == 96)
	{
		mmfirst = false;
		mmtg = 1 - mmtg;

		mmgy = 1;
		mmgx = 0;
		mmdoff = 1;

		mnoise_init(rand());
	}
}

enum MenuMode
{
	MM_SHOW,
	MM_HIDE,
	MM_HIDING,
	MM_SHOWING,
	MM_DISPLAY
};

static const char * EmptyCode = "........";

void mainmenu_open(void)
{
	music_patch_voice3(true);

	music_init(TUNE_MAIN_MENU);

	grid_blank();
	drawBaseGrid();
	status_init();

	mainmenu_anim_init();

	char	state = 0;
	char	passcode[9];
	char	pentry = 0;

	strcpy(passcode, EmptyCode);

	bool	bwait = false;

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

		if (mmfirst)
			state = 1;
		else
			state++;

		tovl_check();

		mainmenu_anim();

		joy_poll(0);
		keyb_poll();

	} while (!joyb[0] && keyb_codes[keyb_key & 0x7f] != ' ' && keyb_codes[keyb_key & 0x7f] != KEY_RETURN);

	MenuMode	mmode = MM_HIDE;
	
	bool	done = false;

	do
	{
		joy_poll(0);
		keyb_poll();

		if (!joyb[0])
			bwait = false;

		switch (mmode)
		{
		case MM_HIDE:
			tovl_hide();
			mmode = MM_HIDING;
			break;
		case MM_HIDING:
			if (tovl_check())
				mmode = MM_SHOW;
			break;
		case MM_SHOW:
		{
			char	buffer[50];
			strcpy(buffer, "LEVEL 00\n");
			strcat(buffer, LevelInfos[game_level].name);
			strcat(buffer, "\nPASSCODE\n");
			if (LevelUnlocked[game_level])
				strcat(buffer, LevelInfos[game_level].passcode);
			else
				strcat(buffer, EmptyCode);
		
			strcpy(passcode, EmptyCode);

			buffer[6] += (game_level + 1) / 10;
			buffer[7] += (game_level + 1) % 10;

			tovl_show(buffer, VCOL_YELLOW);
			tovl_color(0, VCOL_LT_BLUE);
			tovl_color(3, VCOL_LT_BLUE);
			mmode = MM_SHOWING;			
			pentry = 0;
		} break;
		case MM_SHOWING:
			if (tovl_check())
				mmode = MM_DISPLAY;
			break;
		case MM_DISPLAY:
			if (keyb_key & 0x80)
			{
				char c = keyb_codes[keyb_key & 0x7f];

				if (c == KEY_CSR_RIGHT)
				{
					if (game_level < NUM_LEVELS - 1 && LevelUnlocked[game_level])
					{
						game_level++;
						mmode = MM_HIDE;
					}
					
				}
				else if (c == KEY_CSR_LEFT)
				{
					if (game_level > 0)
					{
						game_level--;
						mmode = MM_HIDE;
					}
				}
				else if (c == KEY_DEL && pentry > 0)
				{
					pentry--;
					passcode[pentry] = '.';
					tovl_text(4, passcode);
					tovl_color(4, VCOL_RED);
				}
				else if (c >= 'a' && c <= 'z' && pentry < 8)
				{
					passcode[pentry] = c - 'a' + 'A';
					pentry++;
					tovl_text(4, passcode);
					tovl_color(4, VCOL_RED);

					if (pentry == 8)
					{
						char i = 0;
						while (i < NUM_LEVELS && strcmp(LevelInfos[i].passcode, passcode))
							i++;
						if (i < NUM_LEVELS)
						{
						tovl_color(4, VCOL_WHITE);

							game_level = i;
							mmode = MM_HIDE;
							while (i > 0)
							{
								LevelUnlocked[i] = true;
								i--;
							}
						}
					}
				}
				else if (c == ' ' || c == KEY_RETURN)
				{
					if (LevelUnlocked[game_level])
						done = true;				
				}
			}
			else if (joyb[0] && ! bwait)
			{
				if (LevelUnlocked[game_level])
					done = true;				
			}
			break;
		}

		mainmenu_anim();

	} while (!done);

	tovl_hide();
	tovl_wait();
}
