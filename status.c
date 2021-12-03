#include "status.h"

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
}

void status_update_pos(char x, char y)
{
	status_put_number(0, x);
	status_put_number(3, y);

	char type = gridstate[y][x];

	char i = 0;
	const char * tp = "UNKNOWN"
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

void status_update_state(const char * state)
{

}

void status_update_unit(char unit)
{

}

