#include "battle.h"
#include "window.h"

void battle_init(Battle * b, byte aunit, byte dunit)
{	
	b->units[0] = aunit;
	b->units[1] = dunit;

	byte	dist = unit_distance(aunit, dunit);

	byte	si = 0;

	for(byte t=0; t<2; t++)
	{
		Unit		*	u = units + b->units[t];
		UnitInfo	*	ui = UnitInfos + (u->type & UNIT_TYPE);

		Unit		*	eu = units + b->units[1 - t];
		UnitInfo	*	eui = UnitInfos + (eu->type & UNIT_TYPE);

		byte			nu = u->count, ns = ui->shots >> 4;
		byte			health = (ui->armour & UNIT_INFO_ARMOUR) >> 2;

		if (ui->range & (1 << dist))
		{
			if (eui->range & UNIT_INFO_AIRBORNE)
				b->damage[t] = ui->damage >> 4;
			else
				b->damage[t] = ui->damage & UNIT_INFO_DMG_GROUND;
		}
		else
			b->damage[t] = 0;

		if (b->damage[t] == 0)
			ns = 0;

		for(byte i=0; i<nu; i++)
		{
			b->health[t][i] = health;
			byte	sh = i | (u->type & UNIT_TEAM) | (t ? BATTLE_SHOT_COMBATAND : 0);
			for(byte j=0; j<ns; j++)
				b->shots[si++] = sh;
		}

		for(byte i=nu; i<5; i++)
			b->health[t][i] = 0;
	}

	b->numShots = si;
	b->firedShots = 0;
	b->hitShots = 0;

	for(byte i=0; i<si; i++)
	{
		byte j = rand() % si;
		byte t = b->shots[j]; b->shots[j] = b->shots[i]; b->shots[i] = t;
	}

	window_fill(0x55);
	window_color_rect(0, 0, 5, 10, 0xc2);
	window_color_rect(5, 0, 5, 10, 0xa3);
	for(byte t=0; t<2; t++)
	{
		Unit		*	u = units + b->units[t];
		const char	*	sp = hex_sprite_data(u->type & UNIT_TYPE);

		byte	y = 0;
		for(byte i=0; i<u->count; i++)
		{
			window_put_sprite(4 + 6 * t, y, sp)
			y += 24;
		}
	}
}

bool battle_fire(Battle * b)
{
	byte	f = b->firedShots;
	if (f < b->numShots)
	{
		byte	from = b->shots[f] & BATTLE_SHOT_SRC;
		byte	fi = (b->shots[f] & BATTLE_SHOT_COMBATAND) ? 1 : 0;

		if (b->health[fi][from] > 0)
		{
			byte	to;
			byte	n = 5;
			do 	{
				do
				{
 					to = rand() & 7;
				} while (to > 4);
 				n--;
			} while (n > 0 && b->health[1-fi][to] == 0);
			b->shots[f] |= BATTLE_SHOT_FIRED | (to << 4);
		}
		else
			b->shots[f] = 0;
		f++;
		b->firedShots = f;
	}

	if (f > 8 || f == b->numShots)
	{
		f = b->hitShots;
		if (f < b->numShots)
		{
			if (b->shots[f] & BATTLE_SHOT_FIRED)
			{
				byte	to = (b->shots[f] & BATTLE_SHOT_DST) >> 4;
				byte	ti = (b->shots[f] & BATTLE_SHOT_COMBATAND) ? 0 : 1;

				if (b->health[ti][to] > b->damage[1 - ti])
					b->health[ti][to] -= b->damage[1 - ti];
				else
					b->health[ti][to] = 0;
			}	

			f++;
			b->hitShots = f;
		}
	}

#if 0
	if (b->hitShots == b->numShots)
	{
		for(char i=0; i<2; i++)
		{
			for(char j=0; j<5; j++)
			{
				char buffer[30];
				sprintf(buffer, "%d, %d : %d", i, j, b->health[i][j]);
				window_write(0, 14, buffer);
				window_scroll();				
			}
		}
	}
#endif

	return b->hitShots != b->numShots;
}

void battle_complete(Battle * b)
{
	for(byte t=0; t<2; t++)
	{
		Unit		*	u = units + b->units[t];
		byte			nu = 0;

		for(byte i=0; i<5; i++)
			if (b->health[t][i] > 0)
				nu++;
		u->count = nu;
	}
}
