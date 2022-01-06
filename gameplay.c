#include <c64/joystick.h>
#include <c64/keyboard.h>
#include <c64/vic.h>

#include "gameplay.h"
#include "cursor.h"
#include "hexdisplay.h"
#include "units.h"
#include "window.h"

MovePhases MovePhase;

const byte MovePhaseFlags[8] = {
	MOVPHASE_INTERACTIVE | MOVPHASE_ATTACK,
	MOVPHASE_INTERACTIVE | MOVPHASE_PLAYER | MOVPHASE_TEAM,
	MOVPHASE_ATTACK,
	MOVPHASE_PLAYER | MOVPHASE_TEAM,

	MOVPHASE_INTERACTIVE | MOVPHASE_ATTACK | MOVPHASE_PLAYER | MOVPHASE_TEAM,
	MOVPHASE_INTERACTIVE,
	MOVPHASE_ATTACK | MOVPHASE_PLAYER | MOVPHASE_TEAM,
	0,
};

byte 	joybcount, SelectedUnit;
bool	joyBlockMove;


void game_init(void)
{
	MovePhase = MP_MOVE_2;
	SelectedUnit = 0xff;

}

void game_begin_phase(MovePhases phase)
{
	MovePhase = phase;

	for(char i=0; i<numUnits; i++)
	{
		units[i].tx = units[i].mx;
		units[i].ty = units[i].my;
		units[i].type &= ~UNIT_COMMANDED;
	}

	resetFlags();

	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	calcVisibility(team);
	calcThreatened(team ^ MOVPHASE_TEAM);

	updateColors();
	updateBaseGrid();

	char	color = TeamColors[team ? 1 : 0];

	if (pflags & MOVPHASE_INTERACTIVE)
	{
		if (pflags & MOVPHASE_ATTACK)
		{
			NumBattlePairs = 0;
			status_update_state("ATTACK", color);
		}
		else
			status_update_state("MOVE", color);		
	}
	else
	{
		status_update_state("EXECUTE", color);		
	}
}

void game_execute_battles(void)
{
	Battle	b;

	for(byte bi=0; bi<NumBattlePairs; bi++)
	{
		byte	du = BattlePairs[bi].to;
		if (du != 0xff)
		{
			battle_init(&b, du);
			window_open(4, 4, 12, 15);

			for(byte bj=bi; bj<NumBattlePairs; bj++)
			{
				if (du == BattlePairs[bj].to)
				{
					BattlePairs[bj].to = 0xff;
					battle_begin_attack(&b, BattlePairs[bj].from);

					while (battle_fire(&b))
					{
						for(char phase=0; phase<4; phase++)
						{
							battle_fire_animate(&b, phase);
							vic_waitFrame();
						}
					}

					for(char i=0; i<4; i++)
					{
						for(char phase=0; phase<4; phase++)
						{
							battle_fire_animate(&b, phase);
							vic_waitFrame();
						}				
					}

					battle_complete_attack(&b);
				}
			}

			battle_complete(&b);

			unit_compact();
			drawUnits();

			window_fill(0x00);
			window_close();
		}
	}

	cursor_show();
}

void game_complete_phase(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	if (pflags & MOVPHASE_ATTACK)
	{
		game_execute_battles();
	}

	switch (MovePhase)
	{
		case MP_MOVE_1:
			game_begin_phase(MP_ATTACK_2);
			break;
		case MP_ATTACK_2:
			game_begin_phase(MP_MOVE_2);
			break;
		case MP_MOVE_2:
			game_begin_phase(MP_ATTACK_1);
			break;
		case MP_ATTACK_1:
			game_begin_phase(MP_MOVE_1);
			break;
	}
}

void game_selecthex(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	if (SelectedUnit == 0xff)
	{
		if (gridstate[gridY][gridX] & GS_UNIT)
		{
			byte	unit = gridunits[gridY][gridX];
			if ((units[unit].type & (UNIT_TEAM | UNIT_COMMANDED)) == team)
			{
				if (pflags & MOVPHASE_ATTACK)
				{
					if (calcAttack(unit))
						SelectedUnit = unit;
				}
				else
				{
					if (calcMovement(unit))
						SelectedUnit = unit;
				}
				updateBaseGrid();
			}
		}
	}
	else if (gridstate[gridY][gridX] & GS_SELECT)
	{
		resetMovement();
		units[SelectedUnit].type |= UNIT_COMMANDED;

		if (pflags & MOVPHASE_ATTACK)
		{
			units[SelectedUnit].tx = gridX;
			units[SelectedUnit].ty = gridY;
			battle_add_pair(SelectedUnit, gridunits[gridY][gridX]);
		}
		else
			moveUnit(SelectedUnit, gridX, gridY);

		updateColors();
		updateBaseGrid();
		SelectedUnit = 0xff;
	}
	else
	{
		resetMovement();
		updateBaseGrid();
		SelectedUnit = 0xff;
	}
}

void game_undohex(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	if (gridstate[gridY][gridX] & GS_UNIT)
	{
		char ui = gridunits[gridY][gridX];
		if ((units[ui].type & (UNIT_TEAM | UNIT_COMMANDED)) == (team | UNIT_COMMANDED))
		{
			units[ui].type &= ~UNIT_COMMANDED;
			if (pflags & MOVPHASE_ATTACK)
			{
				moveUnit(ui, units[ui].tx, units[ui].ty);
				battle_cancel_pair(ui);
			}
			updateColors();
			updateBaseGrid();
		}
	}
}

enum JoystickMenu
{
	JM_SELECT,
	JM_DONE,
	JM_UNDO,
	JM_INFO,
	JM_MENU
};

char	keyRepeatDelay;

void game_input(void)
{
	joy_poll(1);

	JoystickMenu menu = JM_SELECT;

	if (joybcount)
	{
		if (joyx[1] | joyy[1])
		{
			if (joyx[1] < 0)
				menu = JM_DONE;
			else if (joyx[1] > 0)
				menu = JM_INFO;
			else if (joyy[1] < 0)
				menu = JM_UNDO;
			else
				menu = JM_MENU;

			spr_image(0, 64 + 17 + menu);
			spr_show(0, true);
			spr_show(2, true);
		}
		else
		{
			spr_show(0, false);
			spr_show(2, false);
		}
	}

	if (joyb[1])
	{
		joybcount++;
	}
	else if (joybcount)
	{
		spr_show(0, false);
		spr_show(2, false);

		joybcount = 0;

		switch (menu)
		{
		case JM_SELECT:
			game_selecthex();
			break;
		case JM_DONE:
			joyBlockMove = true;
			game_complete_phase();
			break;
		case JM_UNDO:
			joyBlockMove = true;
			game_undohex();
			break;
		case JM_INFO:
		case JM_MENU:
			break;
		}
	}
	else if (!joyBlockMove)
	{
		cursor_move(4 * joyx[1], 4 * joyy[1])
	}
	else if (!(joyx[1] | joyy[1]))
	{
		joyBlockMove = false;
	}

	keyb_poll();

	if (keyb_key)
	{
		keyRepeatDelay = 8;

		switch (keyb_codes[keyb_key & 0x7f])
		{
		case ' ':
			game_selecthex();
			break;
		case KEY_CSR_DOWN:
			cursor_move(0, 24);
			break;
		case KEY_CSR_UP:
			cursor_move(0, -24);
			break;
		case KEY_CSR_LEFT:
			cursor_move(-24, 12 - 24 * (gridX & 1));
			break;
		case KEY_CSR_RIGHT:
			cursor_move( 24, 12 - 24 * (gridX & 1));
			break;

		case 'z':
			game_complete_phase();
			break;

		case 'w':
			window_open(4, 4, 13, 8);
			window_write(0, 0, "HELLO WORLD");
			window_write(0, 1, "2ND LINE");
			break;
		case 'e':
			window_close();
			break;
		}
	}
	else if (keyRepeatDelay == 0)
	{
		if (key_pressed(KEY_CODE_CSR_RIGHT))
		{
			if (key_shift())
				cursor_move(-8, 0);
			else
				cursor_move( 8, 0);
		}
		else if (key_pressed(KEY_CODE_CSR_DOWN))
		{
			if (key_shift())
				cursor_move(0, -8);
			else
				cursor_move(0, 8);
		}		
		else
			keyRepeatDelay = 8;
	}
	else
		keyRepeatDelay--;

	if (gridstate[gridY][gridX] & GS_UNIT)
		status_update_unit(gridunits[gridY][gridX]);
	else
		status_update_unit(SelectedUnit);
}
