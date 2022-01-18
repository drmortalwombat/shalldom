#include "battle.h"
#include "window.h"
#include <c64/sprites.h>
#include "hexdisplay.h"


BattlePair	BattlePairs[64];
byte		NumBattlePairs;
byte		Explosion;


void battle_add_pair(byte from, byte to)
{
	BattlePairs[NumBattlePairs].from = from;
	BattlePairs[NumBattlePairs].to = to;
	NumBattlePairs++;
}

void battle_cancel_pair(byte from)
{
	byte	i;
	while (i < NumBattlePairs && BattlePairs[i].from != from)
		i++;
	while (i + 1 < NumBattlePairs)
	{
		BattlePairs[i] = BattlePairs[i + 1];
		i++;
	}
	NumBattlePairs = i;
}

char battle_num_units(Battle * b, byte t)
{
	__assume (t < 2);

	Unit		*	u = units + b->units[t];
	byte			nu = 0;

	for(byte i=0; i<5; i++)
		if (b->health[t][i] > 0)
			nu++;

	return nu;
}

void battle_init_health(Battle * b, byte t)
{
	Unit		*	u = units + b->units[t];
	UnitInfo	*	ui = UnitInfos + (u->type & UNIT_TYPE);
	byte			nu = u->count;
	byte			health = (ui->armour & UNIT_INFO_ARMOUR) >> 2;

	for(byte i=0; i<nu; i++)
		b->health[t][i] = health;
	for(byte i=nu; i<5; i++)
		b->health[t][i] = 0;
}

void battle_init_shots(Battle * b)
{
	unsigned	dist = unit_distance_square(b->units[0], b->units[1]);

	byte	si = 0;
	for(byte t=0; t<2; t++)
	{
		Unit		*	u = units + b->units[t];
		UnitInfo	*	ui = UnitInfos + (u->type & UNIT_TYPE);

		Unit		*	eu = units + b->units[1 - t];
		UnitInfo	*	eui = UnitInfos + (eu->type & UNIT_TYPE);

		byte			ns = ui->shots >> 4;

		unsigned	maxr = hex_size_square(ui->range & UNIT_INFO_SHOT_RANGE);
		unsigned	minr = hex_size_square(ui->range & UNIT_INFO_SHOT_MIN ? 2 : 1);

		if (dist >= minr && dist <= maxr)
		{
			if (eui->view & UNIT_INFO_AIRBORNE)
				b->damage[t] = ui->damage >> 4;
			else
				b->damage[t] = ui->damage & UNIT_INFO_DMG_GROUND;
		}
		else
			b->damage[t] = 0;

		if (b->damage[t] == 0)
			ns = 0;

		for(byte i=0; i<5; i++)
		{
			if (b->health[t][i] > 0)
			{
				byte	sh = i | (u->type & UNIT_TEAM) | (t ? BATTLE_SHOT_COMBATAND : 0);
				for(byte j=0; j<ns; j++)
					b->shots[si++] = sh;
			}
		}
	}

	b->numShots = si;
	b->firedShots = 0;

	// Randomize fire order
	for(byte i=0; i<si; i++)
	{
		byte j = rand() % si;
		byte t = b->shots[j]; b->shots[j] = b->shots[i]; b->shots[i] = t;
	}

}

void battle_init(Battle * b, byte dunit)
{	
	b->units[1] = dunit;

	battle_init_health(b, 1);

	window_color_rect( 0, 0, 12, 15, 0x00);
	window_fill(0x55);

	for(char i=0; i<8; i++)
		spr_set(i, false, 0, 0, 64 + 22, 7, true, false, false);	
}

void battle_begin_attack(Battle * b, byte aunit)
{
	b->units[0] = aunit;

	battle_init_health(b, 0);

	battle_init_shots(b);
}

void battle_enter_units(Battle * b, byte t, byte phase)
{
	Unit		*	u = units + b->units[t];

	if (phase == 0)	
	{
		char color = TeamColors[(u->type & UNIT_TEAM) ? 1 : 0];
		char image = 64 + (u->type & UNIT_TYPE);
		char ca = color | (TerrainColor[gridstate[u->my][u->mx] & GS_TERRAIN] << 4);

		for(char i=3; i<8; i++)
			spr_set(i, true, 0, 0, image, color, true, false, false);		

		window_color_rect( 6 * t, 0, 6, 15, ca);
		window_fill_rect( 6 * t, 0, 6, 15, 0x55);
	}

	if (phase < 16)
	{
		byte	tc = (u->type & UNIT_TEAM) ? 1 : 0;

		int	ux = (u->mx - ox) * 24 + 28, uy = (u->my - oy) * 24 + 50 + 12 * (u->mx & 1);
		int tx = 32 + (winX + 2 + 6 * t) * 8;

		for(byte i=0; i<5; i++)
		{
			if (b->health[t][i])
			{
				int ty = 60 + (winY + 3 * i) * 8;

				spr_show(i + 3, true);
				spr_move(i + 3, ux + ((tx - ux) * phase >> 4), uy + ((ty - uy) * phase >> 4));
			}
		}
	}
	else
	{
		char	ca = TeamColors[(u->type & UNIT_TEAM) ? 1 : 0] | (TerrainColor[gridstate[u->my][u->mx] & GS_TERRAIN] << 4);

		const char	*	sp = hex_sprite_data(u->type & UNIT_TYPE);

		byte	y = 0;
		for(byte i=0; i<5; i++)
		{
			if (b->health[t][i])
				window_put_sprite(2 + 6 * t, y, sp)
			y += 24;
		}

		for(char i=0; i<8; i++)
			spr_set(i, false, 0, 0, 64 + 22, 7, true, false, false);
	}
}

#define SHOTS_IN_FLIGHT	4

bool battle_fire(Battle * b)
{
	spr_show(7, false);

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
	}

	if (f >= SHOTS_IN_FLIGHT)
	{
		f = b->firedShots - SHOTS_IN_FLIGHT;
		if (f < b->numShots)
		{
			if (b->shots[f] & BATTLE_SHOT_FIRED)
			{
				byte	to = (b->shots[f] & BATTLE_SHOT_DST) >> 4;
				byte	ti = (b->shots[f] & BATTLE_SHOT_COMBATAND) ? 0 : 1;

				if (b->health[ti][to] > b->damage[1 - ti])
					b->health[ti][to] -= b->damage[1 - ti];
				else if (b->health[ti][to] > 0)
				{
					b->health[ti][to] = 0;
					window_clear_sprite(2 + 6 * ti, 24 * to, 0x55)
					spr_set(7, true, 24 + (winX + 2 + 6 * ti) * 8, 50 + (winY + 3 * to) * 8, 64 + 24, 7, true, false, false);

					if (!battle_num_units(b, ti))
						b->numShots = b->firedShots;
				}
			}	
		}
	}

	b->firedShots++;

	return b->firedShots < b->numShots + SHOTS_IN_FLIGHT;
}

bool battle_fire_animate(Battle * b, char phase)
{
	char si = 0;
	char step = b->firedShots;
	char from = 0, to = step;
	if (step > SHOTS_IN_FLIGHT)
		from = step - SHOTS_IN_FLIGHT;
	if (to > b->numShots)
		to = b->numShots;

	for(char fi=from; fi<to; fi++)
	{
		if (b->shots[fi] & BATTLE_SHOT_FIRED)
		{
			byte	from = b->shots[fi] & BATTLE_SHOT_SRC;
			byte	fc = (b->shots[fi] & BATTLE_SHOT_COMBATAND) ? 1 : 0;
			byte	to = (b->shots[fi] & BATTLE_SHOT_DST) >> 4;
			byte	tc = (b->shots[fi] & BATTLE_SHOT_COMBATAND) ? 0 : 1;

			byte	pi = (step - fi - 1) * 8 + phase;

			int fy = 60 + (winY + 3 * from) * 8, ty = 60 + (winY + 3 * to) * 8;
			int fx = 32 + (winX + 2 + 6 * fc) * 8, tx = 32 + (winX + 2 + 6 * tc) * 8;

			spr_show(si, true);
			spr_move(si, fx + ((tx - fx) * pi >> 5), fy + ((ty - fy) * pi >> 5));
			si++;
		}
	}
	while (si < SHOTS_IN_FLIGHT)
	{
		spr_show(si, false);
		si++;
	}
	spr_image(7, 64 + 24 + phase);
}

void battle_return_units(Battle * b, byte t)
{
	Unit		*	u = units + b->units[t];

	u->count = battle_num_units(b, t);
}

void battle_complete_attack(Battle * b)
{
	battle_return_units(b, 0);
}


void battle_complete(Battle * b)
{
	battle_return_units(b, 1);
}

