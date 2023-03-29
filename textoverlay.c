#include "textoverlay.h"
#include "splitscreen.h"
#include <c64/vic.h>

static char tovl_last;

// Translate ascii to overlay charset

static char tovl_char(char c)
{
	if (c == ' ')
		return 0 + 88;
	else if (c == '!')
		return 37 + 88;
	else if (c == '?')
		return 38 + 88;
	else if (c == '.')
		return 39 + 88;
	else if (c >= 'A')
		return (c & 0x3f) + 88;
	else if (c >= '0')
		return (c - 21) + 88;
	else
		return 0;
}

void tovl_show(const char * text, char color)
{
	char	mask = 0;
	char	k = 0;

	// Five rows of sprites

	for(char i=0; i<5; i++)
	{
		// Start of row for wipe in/out

		sprovlx[i] = 8 - 2 * i;
		sprovlc[i] = color;

		if (text[k] != '\n')
		{
			for(char j=0; j<8; j++)
			{			
				char c = text[k];
				if (c == ' ')
				{
					c = 0;
					k++;
				}
				else if (c == '!')
				{
					c = 37;
					k++;
				}
				else if (c == '?')
				{
					c = 38;
					k++;
				}
				else if (c == '.')
				{
					c = 39;
					k++;
				}
				else if (c >= 'A')
				{
					c &= 0x3f;
					k++;
				}
				else if (c >= '0')
				{
					c -= 21;
					k++;
				}
				else
					c = 0;

				c += 88;

				// Set char for sprite

				sprovlimg[i][j] = c;
			}
			mask |= 1 << i;
		}
		tovl_last = i;
		if (!text[k])
			break;
		k++;
	}

	// Show all sprites

	vic.spr_enable = 0xff;
	vic.spr_multi = 0xff;
	vic.spr_expand_x = 0x00;
	vic.spr_expand_y = 0x00;
	vic.spr_mcolor1 = VCOL_DARK_GREY;

	// Start sprite ovelay

	split_overlay_show(mask);
}

void tovl_text(char line, const char * text)
{
	// Change text in one line

	char	i = 0;
	while (text[i])
	{
		sprovlimg[line][i] = tovl_char(text[i]);
		i++;
	}

	// Fill with spaces for the rest

	while (i < 8)
	{
		sprovlimg[line][i] = 0;
		i++;		
	}
}

void tovl_color(char line, char color)
{
	sprovlc[line] = color;
}

void tovl_hide(void)
{
	for(int i=0; i<5; i++)
		sprovlx[i] = 34 - 2 * i;
}

bool tovl_check(void)
{
	// Check if overlay is finished moving in/out

	if (tovl_last == 0xff)
		return true;
	else if (sprovlx[tovl_last] == 49)
	{
		vic.spr_enable = 0x00;
		vic.spr_mcolor1 = VCOL_WHITE;

		split_overlay_hide();
		tovl_last = 0xff;

		return true;
	}
	else
		return sprovlx[tovl_last] == 25;
}

void tovl_wait(void)
{
	// Wait for the overlay to complete
	
	if (tovl_last != 0xff)
	{
		while (sprovlx[tovl_last] != 49 && sprovlx[tovl_last] != 25)
			vic_waitFrame();

		tovl_check();
	}
}
