#include "textoverlay.h"
#include "splitscreen.h"
#include <c64/vic.h>

void tovl_show(const char * text, char color)
{
	char	k = 0;
	for(int i=0; i<3; i++)
	{
		sprovlx[i] = 4 - 2 * i;
		for(int j=0; j<8; j++)
		{			
			char c = text[k] & 0x3f;
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
			else if (c >= '0')
			{
				c -= 21;
				k++;
			}
			else if (c != '\n')
				k++;
			else
				c = 0;

			c += 88;

			sprovlimg[i][j] = c;
		}
		k++;
	}

	vic.spr_enable = 0xff;
	vic.spr_multi = 0xff;
	vic.spr_expand_x = 0x00;
	vic.spr_expand_y = 0x00;
	vic.spr_mcolor1 = VCOL_DARK_GREY;

	for(int i=0; i<8; i++)
		vic.spr_color[i] = color;

	split_overlay_show();
}

void tovl_hide(void)
{
	for(int i=0; i<3; i++)
		sprovlx[i] = 25 - 2 * i;

}

void tovl_wait(void)
{
	while (sprovlx[2] != 41 && sprovlx[2] != 20)
		vic_waitFrame();

	if (sprovlx[2] == 41)
	{
		vic.spr_enable = 0x00;
		vic.spr_mcolor1 = VCOL_WHITE;

		split_overlay_hide();	
	}
}
