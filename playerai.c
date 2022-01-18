#include "playerai.h"
#include "units.h"
#include "hexdisplay.h"
#include "battle.h"
#include <c64/vic.h>

char * dbgstr = (char *)0x0400;

void dbglog(const char * str)
{
	while (*str)
		*dbgstr++ = *str++;
	*dbgstr++ = ' ';
}

void dbglogi(int i)
{
	char	buffer[10];
	itoa(i, buffer, 10);
	dbglog(buffer);
}


int playerai_eval_move(byte unit, byte gx, byte gy)
{
	Unit	*	ua = units + unit;
	byte		team = ua->type & UNIT_TEAM;

	int			value = rand() & 3;

	byte	ux = ua->mx, uy = ua->my;

	ua->mx = gx;
	ua->my = gy;


	for(byte j=0; j<numUnits; j++)
	{
		Unit	*	ud = units + j;
		if ((ud->type & UNIT_TEAM) != team)
		{
			sbyte tx = ud->mx, ty = ud->my;
			if (!(gridstate[ty][tx] & GS_HIDDEN))
			{
				if (unit_can_attack(unit, j))
					value += 100;
				if (unit_can_attack(j, unit))
					value -= 50;
			}
		}
	}

	ua->mx = ux; ua->my = uy;

	return value;
}

void playerai_select_move(byte team)
{
	for(byte r=0; r<2; r++)
	{
		for(byte i=0; i<numUnits; i++)
		{
			Unit	*	ua = units + i;
			if ((ua->type & (UNIT_TEAM | UNIT_COMMANDED)) == team)
			{
				calcMovement(i);

				byte	maxx = ua->mx, maxy = ua->my;
				int		maxv = playerai_eval_move(i, maxx, maxy) + 10;

				for(char y=0; y<32; y++)
				{
					for(char x=0; x<32; x++)
					{
						if (gridstate[y][x] & GS_SELECT)
						{
							int	value = playerai_eval_move(i, x, y);
							if (value > maxv)
							{
								maxx = x;
								maxy = y;
								maxv = value;
							}
						}
					}
				}

				if (maxx != ua->mx || maxy != ua->my)
				{
					moveUnit(i, maxx, maxy);
					ua->type |= UNIT_COMMANDED;

					hex_add_path(i);
				}

				resetMovement();
			}
		}
	}
}


void playerai_select_battles(byte team)
{
	// collecting all possible battles

	char battlePairs = 0;

	for(byte i=0; i<numUnits; i++)
	{
		Unit	*	ua = units + i;
		if ((ua->type & UNIT_TEAM) == team)
		{
			UnitInfo	*	info = UnitInfos + (ua->type & UNIT_TYPE);

			for(byte j=0; j<numUnits; j++)
			{
				Unit	*	ud = units + j;
				if ((ud->type & UNIT_TEAM) != team)
				{
					sbyte tx = ud->mx, ty = ud->my;
					if (!(gridstate[ty][tx] & GS_HIDDEN))
					{
						int	value = unit_attack_value(i, j);

						if (value > 0)
						{
							value -= unit_attack_value(j, i);

							BattlePairs[battlePairs].from = i;
							BattlePairs[battlePairs].to = j;
							BattlePairs[battlePairs].value = value;
							battlePairs++;
						}
					}
				}
			}
		}
	}

	NumBattlePairs = 0;
	while (NumBattlePairs < battlePairs)
	{
		int		maxv = -32768;
		char	maxp = 0;

		for(char i=NumBattlePairs; i<battlePairs; i++)
		{
			if (BattlePairs[i].value > maxv)
			{
				maxp = i;
				maxv = BattlePairs[i].value;
			}
		}

		BattlePair	p = BattlePairs[maxp];
		BattlePairs[maxp] = BattlePairs[NumBattlePairs];
		BattlePairs[NumBattlePairs] = p;
		NumBattlePairs++;

		char	np = NumBattlePairs;
		for(char i=NumBattlePairs; i<battlePairs; i++)
		{
			if (BattlePairs[i].from != p.from)
			{
				BattlePairs[np] = BattlePairs[i];
				np++;
			}
		}
		battlePairs = np;
	}
}
