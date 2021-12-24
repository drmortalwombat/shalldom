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
		units[i].type &= ~UNIT_MOVED;
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
			status_update_state("ATTACK", color);
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
	for(byte ui=0; ui<numUnits; ui++)
	{
		if (units[ui].type & UNIT_MOVED)
		{
			byte tx = units[ui].tx;
			byte ty = units[ui].ty;

			byte uj = gridunits[ty][tx];

			window_open(4, 4, 20, 15);			
			Battle	b;
			memset(&b, 0xff, sizeof(b));
			battle_init(&b, ui, uj);
			while (battle_fire(&b))
				;
			battle_complete(&b);
		}
	}
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
			if ((units[unit].type & (UNIT_TEAM | UNIT_MOVED)) == team)
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
		units[SelectedUnit].type |= UNIT_MOVED;

		if (pflags & MOVPHASE_ATTACK)
		{
			units[SelectedUnit].tx = gridX;
			units[SelectedUnit].ty = gridY;
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
		if ((units[ui].type & (UNIT_TEAM | UNIT_MOVED)) == (team | UNIT_MOVED))
		{
			units[ui].type &= ~UNIT_MOVED;
			if (pflags & MOVPHASE_ATTACK)
				moveUnit(ui, units[ui].tx, units[ui].ty);
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

			Screen[0x03f8] = 64 + 17 + menu;
			vic.spr_enable |= 5;
		}
		else
			vic.spr_enable &= 2;
	}

	if (joyb[1])
	{
		joybcount++;
	}
	else if (joybcount)
	{
		vic.spr_enable &= 2;
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

	if (gridstate[gridY][gridX] & GS_UNIT)
		status_update_unit(gridunits[gridY][gridX]);
	else
		status_update_unit(SelectedUnit);
}
