#include "playerai.h"
#include "units.h"
#include "hexdisplay.h"
#include "battle.h"
#include <c64/vic.h>

AITask	*	AITasks;

unsigned tsqrt(unsigned n)
{
    unsigned p, q, r, h

    p = 0;
    r = n;

#assign q 0x4000
#repeat
    {
        h = p | q;
        p >>= 1;
        if (r >= h)
        {
            p |= q;
            r -= h;
        } 
    }
#assign q q >> 2
#until q == 0
#undef q

    return p;
}

int playerai_eval_move(byte unit, byte gx, byte gy, const AITask * task)
{
	Unit		*	ua = units + unit;
	UnitInfo	*	ui = UnitInfos + (ua->type & UNIT_TYPE);
	byte			team = ua->type & UNIT_TEAM;

	int				value = rand() & 3;

	byte			ux = ua->mx, uy = ua->my;

	ua->mx = gx;
	ua->my = gy;

	if (!(ui->view & UNIT_INFO_AIRBORNE))
	{
		byte	terrain = gridstate[gy][gx] & GS_TERRAIN;
		byte	dugin = 0;

		if (UnitInfos[ua->type & UNIT_TYPE].armour & UNIT_INFO_DIG_IN)
		{
			if (ux == gx && uy == gy)
				dugin = 1;
		}
		value += ground_agility[dugin][terrain] * 16;
	}

	AIStrategy	ais = task->strategy & AI_STRATEGY;

	unsigned dd = hex_dist_square(gx, gy, task->mx, task->my);

	int	attscore = 100, defscore = -50;

	switch (ais)
	{
	case AIS_RUSH:
		value += 4096 - tsqrt(dd) * 32;
		attscore = 20;
		break;
	case AIS_STROLL:
		value += 4096 - tsqrt(dd) * 32;
		break;
	}

	if (UnitInfos[ua->type & UNIT_TYPE].range & UNIT_INFO_SHOT_DELAY)
	{
		if (ux != gx || uy != gy)
			attscore >>= 2;
	}

	for(byte j=0; j<numUnits; j++)
	{
		Unit	*	ud = units + j;
		if ((ud->type & UNIT_TEAM) != team)
		{
			sbyte tx = ud->mx, ty = ud->my;
			if (!(gridstate[ty][tx] & GS_HIDDEN))
			{
				if (unit_can_attack(unit, j))
					value += attscore;
				if (unit_can_attack(j, unit))
					value += defscore;
			}
		}
	}

	ua->mx = ux; ua->my = uy;

	return value;
}

void playerai_advance(byte team)
{
	for(byte i=0; i<numUnits; i++)
	{			
		Unit	*	ua = units + i;
		if ((ua->type & (UNIT_TEAM | UNIT_COMMANDED)) == team)
		{
			const AITask	*	task = AITasks + (ua->id >> 3);
			bool				advance = false;

			if (task->timeout == GameDays)
				advance = true;
			else
			{
				AIStrategy	ais = task->strategy & AI_STRATEGY;
				sbyte		dx = task->mx - ua->mx, dy = task->my - ua->my;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;
				byte	dd = dx;
				if (dy > dx)
					dd = dy;

				switch (ais)
				{
				case AIS_RUSH:
				case AIS_STROLL:
					if (dd <= 1)
						advance = true;
					break;
				}
			}

			if (advance)
				ua->id = (ua->id & UNIT_ID_DIVISION) | (task->strategy & AI_TASK_ID);
		}
	}
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
				const AITask	*	task = AITasks + (ua->id >> 3);

				calcMovement(i);

				byte	maxx = ua->mx, maxy = ua->my;
				int		maxv = playerai_eval_move(i, maxx, maxy, task) + 10;

				for(char y=0; y<32; y++)
				{
					for(char x=0; x<32; x++)
					{
						if (gridstate[y][x] & GS_SELECT)
						{
							int	value = playerai_eval_move(i, x, y, task);
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
				else
				{
					ua->type |= UNIT_COMMANDED;
					ua->flags |= UNIT_FLAG_REPAIR;
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
	char tcnt[32];

	for(byte i=0; i<32; i++)
		tcnt[i] = 0;

	for(byte i=0; i<numUnits; i++)
	{
		Unit	*	ua = units + i;
		if ((ua->type & UNIT_TEAM) == team)
		{
			ua->flags |= UNIT_FLAG_REPAIR;

			UnitInfo	*	info = UnitInfos + (ua->type & UNIT_TYPE);

			for(byte j=0; j<numUnits; j++)
			{
				Unit	*	ud = units + j;
				if ((ud->type & UNIT_TEAM) != team)
				{
					if (!(gridstate[ud->my][ud->mx] & GS_HIDDEN))
					{
						int	value = unit_attack_value(i, j, false);

						if (value > 0)
						{
							value -= unit_attack_value(j, i, true);

							BattlePairs[battlePairs].from = i;
							BattlePairs[battlePairs].to = j;
							BattlePairs[battlePairs].value = value;
							battlePairs++;

							tcnt[j]++;
						}
					}
				}
			}
		}
	}

	// Hand out bonus for multi target

	for(byte i=0; i<battlePairs; i++)
		BattlePairs[i].value += 4 << tcnt[BattlePairs[i].to];

	// Now pick the best battles for each unit
	
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

		if (maxv < -32)
			break;

		BattlePair	p = BattlePairs[maxp];
		BattlePairs[maxp] = BattlePairs[NumBattlePairs];
		BattlePairs[NumBattlePairs] = p;
		NumBattlePairs++;

		units[p.from].flags &= ~UNIT_FLAG_REPAIR;

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
