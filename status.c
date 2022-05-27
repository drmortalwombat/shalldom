#include "status.h"
#include <c64/vic.h>

void status_put_number(char pos, char val)
{
	char	c = '0';

	if (val >= 50)
	{
		c += 5; val -= 50;
	}
	while (val >= 20)
	{
		c += 2;
		val -= 20;
	}
	if (val >= 10)
	{
		c += 1;
		val -= 10;
	}

	Screen[24 * 40 + pos] = c;
	Screen[24 * 40 + 1 + pos] = '0' + val;
}

void status_init(void)
{
	memset(Screen + 24 * 40, ' ', 40);
	memset(Color + 24 * 40, VCOL_LT_GREY, 40);
}

void status_show_progress(const char * action, byte progress)
{
	char i, p;

	i = 0;
	while (action[i])
	{
		Screen[24 * 40 + i] = action[i];
		Color[24 * 40 + i] = VCOL_WHITE;
		i++;
	}

	while (i < 8)
	{
		Screen[24 * 40 + i] = ' ';
		i++;
	}

	p = 0;
	while (p < progress)
	{
		Screen[24 * 40 + i] = 0x2a;
		Color[24 * 40 + i] = VCOL_YELLOW;
		p++;
		i++;		
	}

	while (i < 40)
	{
		Screen[24 * 40 + i] = 0x2a;
		Color[24 * 40 + i] = VCOL_BLUE;
		i++;
	}
}


void status_update_pos(char x, char y)
{
	status_put_number(0, x);
	status_put_number(3, y);

	char type = gridstate[y][x];

	char i = 0;
	const char * tp = S"UNKNOWN"
	if (!(type & GS_HIDDEN))
		tp = TerrainNames[type & GS_TERRAIN];

	while (tp[i])
	{
		Screen[24 * 40 + 6 + i] = tp[i];
		i++;
	}
	while (i < 8)
	{
		Screen[24 * 40 + 6 + i] = ' ';
		i++;
	}
}

#define UNIT_COL	22
#define STATE_COL	15

void status_update_state(const char * state, byte color)
{
	char i = 0;
	while (state[i])
	{
		Screen[24 * 40 + STATE_COL + i] = state[i];
		i++;
	}
	while (i < 7)
	{
		Screen[24 * 40 + STATE_COL + i] = ' ';
		i++;
	}

	for(char i=0; i<7; i++)
		Color[24 * 40 + STATE_COL + i] = color;
}

const char nthnames[] = S"STNDRDTHTH";

void status_update_unit(char unit)
{
	if (unit != 0xff)
	{
		char division = units[unit].id & UNIT_ID_DIVISION;

		Screen[24 * 40 + UNIT_COL + 0] = '1' + division;
		if (division > 4)
			division = 4;
		Screen[24 * 40 + UNIT_COL + 1] = 0x3c + division;
		const char * tp = UnitInfos[units[unit].type & UNIT_TYPE].name;
		char i = 0;
		while (tp[i])
		{
			Screen[24 * 40 + UNIT_COL + 3 + i] = tp[i];
			i++;
		}
		Screen[24 * 40 + UNIT_COL + 3 + i] = ' ';
		Screen[24 * 40 + UNIT_COL + 4 + i] = '0' + units[unit].count;
		Screen[24 * 40 + UNIT_COL + 5 + i] = 0x1b + ((units[unit].flags & UNIT_FLAG_EXPERIENCE) >> 5);

		while (i < 12)
		{
			Screen[24 * 40 + UNIT_COL + 6 + i] = ' ';
			i++;
		}

		char color = TeamColors[units[unit].type & UNIT_TEAM ? 1 : 0];
		for(char i=0; i<18; i++)
			Color[24 * 40 + UNIT_COL + i] = color;
	}
	else
	{
		for(char i=0; i<18; i++)
			Screen[24 * 40 + UNIT_COL + i] = ' ';
	}
}

