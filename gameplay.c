#include <c64/joystick.h>
#include <c64/keyboard.h>
#include <c64/vic.h>

#include "gameplay.h"
#include "cursor.h"
#include "hexdisplay.h"
#include "units.h"

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

const char * PhaseNames[8] = {
	"ATTACK PLAYER 1",
	"MOVE PLAYER 2",
	"ATTACKING PLAYER 1",
	"MOVING PLAYER 2",
	"ATTACK PLAYER 2",
	"MOVE PLAYER 1",
	"ATTACKING PLAYER 2",
	"MOVING PLAYER 1"
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

	byte team = MovePhaseFlags[MovePhase] & MOVPHASE_TEAM;

	calcVisibility(team);
	calcThreatened(team ^ MOVPHASE_TEAM);

	updateColors();
	updateBaseGrid();	
}

void game_complete_phase(void)
{
	switch (MovePhase)
	{
		case MP_MOVE_1:
			game_begin_phase(MP_MOVE_2);
			break;
		case MP_MOVE_2:
			game_begin_phase(MP_MOVE_1);
			break;
	}
}

void game_selecthex(void)
{
	byte team = MovePhaseFlags[MovePhase] & MOVPHASE_TEAM;

	if (SelectedUnit == 0xff)
	{
		if (gridstate[gridY][gridX] & GS_UNIT)
		{
			SelectedUnit = gridunits[gridY][gridX];
			if ((units[SelectedUnit].type & (UNIT_TEAM | UNIT_MOVED)) == team)
			{
				calcMovement(SelectedUnit);
				updateBaseGrid();
			}
			else
				SelectedUnit = 0xff;
		}
	}
	else if (gridstate[gridY][gridX] & GS_SELECT)
	{
		resetMovement();
		units[SelectedUnit].type |= UNIT_MOVED;
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
	byte team = MovePhaseFlags[MovePhase] & MOVPHASE_TEAM;

	vic.color_border++;
	if (gridstate[gridY][gridX] & GS_UNIT)
	{
		char ui = gridunits[gridY][gridX];
		if ((units[ui].type & (UNIT_TEAM | UNIT_MOVED)) == (team | UNIT_MOVED))
		{
			units[ui].type &= ~UNIT_MOVED;
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
		}
	}

}
