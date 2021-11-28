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

byte joybcount, SelectedUnit;


void game_init(void)
{
	MovePhase = MP_MOVE_2;
	SelectedUnit = 0xff;
}

void game_selecthex(void)
{
	if (SelectedUnit == 0xff)
	{
		if (gridstate[gridY][gridX] & GS_UNIT)
		{
			SelectedUnit = gridunits[gridY][gridX];
			if ((units[SelectedUnit].type & UNIT_TEAM) == UNIT_TEAM_1)
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
		moveUnit(SelectedUnit, gridX, gridY);

		resetFlags();
		calcVisibility(UNIT_TEAM_1);
		calcThreatened(UNIT_TEAM_2);

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

void game_input(void)
{
	joy_poll(1);

	if (joyb[1])
	{
		joybcount++;
		if (joyx[1] | joyy[1])
		{
			vic.spr_enable |= 6;
		}
		else
			vic.spr_enable &= 1;
	}
	else if (joybcount)
	{
		joybcount = 0;
		game_selecthex();
	}
	else
	{
		cursor_move(4 * joyx[1], 4 * joyy[1])
	}

	keyb_poll();

	if (keyb_key)
	{
		switch (keyb_codes[keyb_key & 0x7f])
		{
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
