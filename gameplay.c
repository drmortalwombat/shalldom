#include <c64/joystick.h>
#include <c64/keyboard.h>
#include <c64/vic.h>
#include <audio/sidfx.h>

#include "gameplay.h"
#include "cursor.h"
#include "hexdisplay.h"
#include "units.h"
#include "window.h"
#include "playerai.h"
#include "hexmap.h"
#include "gamemusic.h"
#include "levels.h"

MovePhases MovePhase, NextPhase;

GamePhases GamePhase;
char GameDays;

// Movement phase flags
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

// Soud effects

const SIDFX	SIDFXDing[1] = {{
	6400, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x20  | SID_DKY_204,
	0, 0,
	1, 10,
	1
}};

const SIDFX	SIDFXDing2[1] = {{
	3200, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x80  | SID_DKY_204,
	0, 0,
	2, 10,
	1
}};

const SIDFX	SIDFXFail[1] = {{
	800, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x80  | SID_DKY_300,
	0, 0,
	2, 10,
	1
}};

const SIDFX	SIDFXUndo[2] = {{
	3200, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x80  | SID_DKY_204,
	200, 0,
	8, 0,
	1
},{
	3200, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x80  | SID_DKY_204,
	-200, 0,
	2, 10,
	1
}};

const SIDFX	SIDFXTankMove[1] = {{
	800, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_100 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-50, 0,
	10, 0,
	2
}};

const SIDFX	SIDFXHovercraftMove[1] = {{
	800, 1000, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_100 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	0, -80,
	10, 0,
	2
}};

const SIDFX	SIDFXBomberMove[1] = {{
	500, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_750,
	0, 0,
	14, 0,
	2
}};

const SIDFX	SIDFXChopperMove[3] = {{
	3200, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_100 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-400, 0,
	5, 1,
	2
},{
	3200, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_100 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-400, 0,
	5, 1,
	2
},{
	3200, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_100 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-400, 0,
	5, 1,
	2
}};

const SIDFX	SIDFXDroneMove[6] = {{
	6400, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_68 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-800, 0,
	2, 1,
	2
},{
	6400, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_68 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-800, 0,
	2, 1,
	2
},{
	6400, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_68 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-800, 0,
	2, 1,
	2
},{
	6400, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_68 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-800, 0,
	2, 1,
	2
},{
	6400, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_68 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-800, 0,
	2, 1,
	2
},{
	6400, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_68 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-800, 0,
	2, 1,
	2
}};

const SIDFX	SIDFXInfantryMove[4] = {{
	2000, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-200, 0,
	1, 3,
	2
},{
	2050, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-200, 0,
	1, 3,
	2
},{
	1930, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-200, 0,
	1, 3,
	2
},{
	1970, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_2 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-200, 0,
	1, 3,
	2
}};

void sidfx_play_fail(void)
{
	sidfx_play(2, SIDFXFail, 1);
}

void sidfx_play_undo(void)
{
	sidfx_play(2, SIDFXUndo, 2);
}

void sidfx_play_step(void)
{
	sidfx_play(2, SIDFXDing, 1);
}

void sidfx_play_select(void)
{
	sidfx_play(2, SIDFXDing2, 1);
}

// Play movement sound effect for a unit
void sidfx_play_move(byte unit)
{
	if (unit == UNIT_INFANTRY)
		sidfx_play(2, SIDFXInfantryMove, 4);	
	else if (unit == UNIT_CHOPPER)
		sidfx_play(2, SIDFXChopperMove, 3);	
	else if (unit == UNIT_SCOUT_DRONE)
		sidfx_play(2, SIDFXDroneMove, 6);	
	else if (unit == UNIT_BOMBER)
		sidfx_play(2, SIDFXBomberMove, 6);	
	else if (unit == UNIT_HOVERCRAFT)
		sidfx_play(2, SIDFXHovercraftMove, 1);	
	else
		sidfx_play(2, SIDFXTankMove, 1);	
}

void game_init(void)
{
	GamePhase = GP_TITLE;
}

static const char * PhaseOverlay[] = 
{
	" PLAYER\n ATTACK",
	" ENEMY\n MOVES",
	nullptr,
	nullptr,
	" ENEMY\nATTACKS",
	" PLAYER\n MOVE",
	nullptr,
	nullptr,
};

void game_invoke_playerai(void);

// Show overlay message at phase start

void game_show_overlay(void)
{
	const char * po = PhaseOverlay[MovePhase];
	if (po)
	{
		char	buffer[32];
		char	n = 0;
		while (po[n])
		{
			buffer[n] = po[n];
			n++;
		}
		buffer[n++] = '\n';
		char	d = GameDays;
		char	e = '0';

		if (d >= 20)
		{
			e += 2;
			d -= 20;
		}
		if (d >= 10)
		{
			e += 1;
			d -= 10;
		}
		
		if (e != '0')
			buffer[n++]	= e;
		else
			buffer[n++]	= ' ';

		buffer[n++] = d + '0';
		buffer[n++] = ' ';
		buffer[n++] = 'D';
		buffer[n++] = 'A';
		buffer[n++] = 'Y';
		buffer[n++] = 'S';
		buffer[n++] = 0;

		tovl_show(buffer, TeamColors[(MovePhaseFlags[MovePhase] & MOVPHASE_TEAM) ? 1 : 0]);
	}
}

void game_begin_phase(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	// Interactive phase requires an overlay for the user

	if (pflags & MOVPHASE_INTERACTIVE)
		game_show_overlay();

	// Reset commands for all units
	for(char i=0; i<numUnits; i++)
	{
		units[i].tx = units[i].mx;
		units[i].ty = units[i].my;
		units[i].type &= ~UNIT_COMMANDED;
		ghostUnit(i);
	}

	resetFlags();

	byte 	team = pflags & MOVPHASE_TEAM;

	if (pflags & MOVPHASE_INTERACTIVE)
	{
		calcVisibility(team);
		calcThreatened(team ^ MOVPHASE_TEAM);
	}
	else
		calcVisibility(0x00);

	if (team)
	{
		if (!(pflags & MOVPHASE_INTERACTIVE))
			updateBaseGrid();
	}
	else
		grid_redraw_all();

	char	color = TeamColors[team ? 1 : 0];

	if (pflags & MOVPHASE_INTERACTIVE)
	{		
		tovl_wait();

		// Show overlay
		if (!team)
		{
			for(char i=0; i<30; i++)
				vic_waitFrame();

			tovl_hide();
			tovl_wait();
			cursor_show();		
		}

		// Show status line
		if (pflags & MOVPHASE_ATTACK)
		{
			NumBattlePairs = 0;
			status_update_state(S"ATTACK", color);	
		}
		else
		{
			NumPaths = 0;
			status_update_state(S"MOVE", color);		
		}

	}
	else
	{
		cursor_hide();
		status_update_state(S"EXECUTE", color);
	}
}

// Execute all pending battles

void game_execute_battles(void)
{
	if (NumBattlePairs == 0)
		return;

	// Init battle music for attacking team

	if (MovePhaseFlags[MovePhase] & MOVPHASE_TEAM)
		music_init(rand() & 1 ? TUNE_DEFEND_1 : TUNE_DEFEND_2);
	else
		music_init(rand() & 1 ? TUNE_ATTACK_1 : TUNE_ATTACK_2);

	Battle	b;

	for(byte bi=0; bi<NumBattlePairs; bi++)
	{
		byte	du = BattlePairs[bi].to;
		if (du != 0xff)
		{
			// Scroll attacked unit into view

			hex_scroll_into_view(du);
	
			char ux = units[du].mx - ox;

			// Open window on other half of screen, so it does not
			// occupy the attacked unit

			if (ux < 7)
				window_open(24, 4, 12, 15);
			else
				window_open(4, 4, 12, 15);

			// Disable repair command for attacked unit

			units[du].flags &= ~UNIT_FLAG_REPAIR;

			// Init battle

			battle_init(&b, du);

			sidfx_play_move(units[du].type & UNIT_TYPE);

			// Move defending units into batle window

			for(char phase=0; phase<=16; phase++)
			{
				battle_enter_units(&b, CBT_DEFENDER, phase);
				vic_waitFrame();
			}

			// Iterate over all battles that include this unit as defender

			for(byte bj=bi; bj<NumBattlePairs; bj++)
			{
				if (du == BattlePairs[bj].to)
				{
					BattlePairs[bj].to = 0xff;

					// Check if defender is still standing

					if (battle_num_units(&b, CBT_DEFENDER))
					{
						byte	au = BattlePairs[bj].from;

						battle_begin_attack(&b, au);

						sidfx_play_move(units[au].type & UNIT_TYPE);

						// Move attacking units into window

						for(char phase=0; phase<=16; phase++)
						{
							battle_enter_units(&b, CBT_ATTACKER, phase);
							vic_waitFrame();
						}

						// Have all units fire

						while (battle_fire(&b))
						{
							for(char phase=0; phase<8; phase++)
							{
								battle_fire_animate(&b, phase);
								vic_waitFrame();
							}
						}

						// Complete final shots and explosions
						for(char i=0; i<4; i++)
						{
							for(char phase=0; phase<8; phase++)
							{
								battle_fire_animate(&b, phase);
								vic_waitFrame();
							}				
						}

						// Return remaining attacking units from battle

						battle_complete_attack(&b);
					}
				}
			}

			// Finish battle

			battle_complete(&b);

			window_fill(0x00);
			window_close();

			// Draw remaining units

			drawUnits();
		}
	}

	// Remove destroyed squads 

	unit_compact();
	drawUnits();

	// Restart normal game tune

	int score = level_eval_score();

	if (score <= -400)
		music_queue(TUNE_THEME_LOOSING);
	else if (score >= 400)
		music_queue(TUNE_THEME_WINNING);
	else
		music_queue(TUNE_THEME_NEUTRAL);
}

// Execute repair command for all scheduled squads

void game_execute_repair(void)
{
	for(char i=0; i<numUnits; i++)
	{
		if (units[i].flags & UNIT_FLAG_REPAIR)
		{
			// Repair one unit per squad
			if (units[i].count < 5)
				units[i].count++;
		}
		units[i].flags &= ~UNIT_FLAG_REPAIR;
	}
}

void game_swap_moves(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	for(char i=0; i<numUnits; i++)
	{
		if ((units[i].type & (UNIT_TEAM | UNIT_COMMANDED)) == (team | UNIT_COMMANDED))
			moveUnit(i, units[i].tx, units[i].ty);
	}

//	updateBaseGrid();
}

// Execute movement phase

void game_execute_moves(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	// Set the rested flag for all units (will be reset, when moved)

	for(char i=0; i<numUnits; i++)
	{
		if ((units[i].type & UNIT_TEAM ) == team)
			units[i].flags |= UNIT_FLAG_RESTED;
	}

	// Iterate over all planned paths

	for(char pi=0; pi<NumPaths; pi++)
	{
		Path	*	p = Paths + pi;
		char		x = p->sx, y = p->sy;

		if (gridstate[y][x] & GS_UNIT)
		{
			char		ui = gridunits[y][x];

			// Scroll non hidden units into view

			if (!(gridstate[y][x] & GS_HIDDEN))
				hex_scroll_into_view(ui);

			Unit	*	u = units + ui;

			// Reset rested flsg

			u->flags &= ~UNIT_FLAG_RESTED;

			char color = TeamColors[(u->type & UNIT_TEAM) ? 1 : 0];
			char image = 48 + (u->type & UNIT_TYPE);

			// Put sprite at start position

			spr_set(3, !(gridstate[y][x] & GS_HIDDEN), 0, 0, image, color, true, false, false);

			// Remove sprite pixels from bitmap

			hideUnit(ui);
			updateGridCell(x, y);

			// find first step (last in list)

			char pi = 8;
			while (p->steps[pi - 1] == 0xff)
				pi--;

			do  {
				sidfx_play_move(u->type & UNIT_TYPE);

				pi--;
				byte	state = gridstate[y][x];

				// Calculate sprite path

				int	ux = (x - ox) * 24 + 28, uy = (y - oy) * 24 + 50 + 12 * (x & 1);
				char	d = p->steps[pi];
				y = (2 * y + (x & 1) - PathY[d]) >> 1;
				x -= PathX[d];
				int	tx = (x - ox) * 24 + 28, ty = (y - oy) * 24 + 50 + 12 * (x & 1);

				state &= gridstate[y][x];

				bool visible = !(state & GS_HIDDEN);
				if (visible)
				{
					// Move sprite along path
					spr_show(3, true);
					for(char j=0; j<16; j++)
					{
						spr_move16(3, ux + ((tx - ux) * j >> 4), uy + ((ty - uy) * j >> 4));
						vic_waitFrame();
					}
				}
				else
					spr_show(3, false);

			} while (pi > 0);

			// Put image back into bitmap

			moveUnit(ui, x, y);
			updateGridCell(x, y);

			// Hide sprite

			spr_show(3, false);
		}
	}
}

// Complete movement phase

void game_complete_phase(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	if (pflags & MOVPHASE_INTERACTIVE)
	{
		if (!(pflags & MOVPHASE_TEAM))
		{
			resetMovement();
			updateBaseGrid();
			SelectedUnit = 0xff;
		}
		else
		{
			tovl_hide();
			tovl_wait();
		}

		if (pflags & MOVPHASE_ATTACK)
		{

		}
		else
		{
			game_swap_moves();			
		}
	}
	else
	{
		if (pflags & MOVPHASE_ATTACK)
		{
			game_execute_battles();
		}
		else
		{
			game_execute_moves();
		}
		game_execute_repair();
	}

	// Advance to next phase

	NextPhase = (MovePhase + 1) & 7;

	if (NextPhase == 0)
		GameDays--;
}

// Have AI decide next move for computer plaer

void game_invoke_playerai(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte	team = pflags & MOVPHASE_TEAM;

	// Advance scripted AI

	playerai_advance(team);

	// Use non scripted tactical AI

	if (pflags & MOVPHASE_ATTACK)
	{
		playerai_select_battles(team);
	}
	else
	{
		playerai_select_move(team);		
	}

	game_complete_phase();
}

// Player selected a hex field

void game_selecthex(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	// Is no unit selected ?

	if (SelectedUnit == 0xff)
	{		
		if (gridstate[gridY][gridX] & GS_UNIT)
		{
			byte	unit = gridunits[gridY][gridX];
			if ((units[unit].type & (UNIT_TEAM | UNIT_COMMANDED)) == team)
			{
				// Select unit

				if (pflags & MOVPHASE_ATTACK)
				{
					if (calcAttack(unit))
					{
						sidfx_play_select();
						SelectedUnit = unit;
					}
					else
						sidfx_play_fail();
				}
				else
				{
					if (calcMovement(unit))
					{
						sidfx_play_select();
						SelectedUnit = unit;
					}
					else
						sidfx_play_fail();
				}
				updateBaseGrid();
			}
			else
				sidfx_play_fail();
		}
		else
			sidfx_play_fail();	
	}
	else if (gridstate[gridY][gridX] & GS_SELECT)
	{
		// Remember target for selected squad and mark as commanded

		units[SelectedUnit].type |= UNIT_COMMANDED;

		sidfx_play_select();
		if (pflags & MOVPHASE_ATTACK)
		{
			units[SelectedUnit].tx = gridX;
			units[SelectedUnit].ty = gridY;
			battle_add_pair(SelectedUnit, gridunits[gridY][gridX]);
			ghostUnit(SelectedUnit);
		}
		else
		{
			moveUnit(SelectedUnit, gridX, gridY);
			hex_add_path(SelectedUnit);
		}

		resetMovement();
		updateBaseGrid();
		SelectedUnit = 0xff;
	}
	else
	{
		// Play a fail sound

		sidfx_play_fail();

		resetMovement();
		updateBaseGrid();
		SelectedUnit = 0xff;
	}
}

// Repair squad on selected hex field

void game_repairhex(void)
{
	if (SelectedUnit == 0xff)
	{
		byte	pflags = MovePhaseFlags[MovePhase];
		byte 	team = pflags & MOVPHASE_TEAM;

		if (gridstate[gridY][gridX] & GS_UNIT)
		{
			char ui = gridunits[gridY][gridX];
			if ((units[ui].type & (UNIT_TEAM | UNIT_COMMANDED)) == team)
			{
				// Set repair flag, and mark unit as commanded

				units[ui].type |= UNIT_COMMANDED;
				units[ui].flags |= UNIT_FLAG_REPAIR;
				ghostUnit(ui);
				updateBaseGrid();
			}
			else
				sidfx_play_fail();
		}
		else
			sidfx_play_fail();
	}
	else
		sidfx_play_fail();
}

// Undo command plan for unit on selected hex field

void game_undohex(void)
{
	if (SelectedUnit == 0xff)
	{
		byte	pflags = MovePhaseFlags[MovePhase];
		byte 	team = pflags & MOVPHASE_TEAM;

		if (gridstate[gridY][gridX] & GS_UNIT)
		{
			char ui = gridunits[gridY][gridX];
			if ((units[ui].type & (UNIT_TEAM | UNIT_COMMANDED)) == (team | UNIT_COMMANDED))
			{
				sidfx_play_undo();

				// Reset command flag and remove target from list

				units[ui].type &= ~UNIT_COMMANDED;
				if (units[ui].flags & UNIT_FLAG_REPAIR)
				{
					units[ui].flags &= ~UNIT_FLAG_REPAIR;
					ghostUnit(ui);
				}
				else if (pflags & MOVPHASE_ATTACK)
				{
					battle_cancel_pair(ui);
					ghostUnit(ui);
				}
				else
				{
					hex_cancel_path(ui);
					moveUnit(ui, units[ui].tx, units[ui].ty);
				}			
				updateBaseGrid();
			}
			else
				sidfx_play_fail();
		}
		else
			sidfx_play_fail();
	}
	else
		sidfx_play_fail();
}

enum JoystickMenu
{
	JM_SELECT,
	JM_MAP,
	JM_REPAIR,
	JM_DONE,
	JM_UNDO,
	JM_INFO,
	JM_MENU,
};

char	keyRepeatDelay;

// Check for a user interaction that ends a menu or
// advances to the next phase

bool game_check_continue(void)
{
	joy_poll(0);
	if (joyb[0])
		return true;
	keyb_poll();
	switch (keyb_codes[keyb_key & 0x7f])
	{
		case ' ':
		case '\n':
		case '\r':
			return true;
	}

	return false;
}

// Show unit info

void game_show_info(void)
{
	if (gridstate[gridY][gridX] & GS_UNIT)
	{
		char ui = gridunits[gridY][gridX];

		cursor_hide();

		window_open(4, 4, 18, 12);

		unit_showinfo(ui, 0);

		while (!game_check_continue())
			vic_waitFrame();

		window_close();

		cursor_show();
	}

}

// Show minimap

void game_show_map(void)
{
	cursor_hide();

	// Open window

	window_open(4, 4, 16, 16);

	window_color_rect(0, 0, 16, 16, (VCOL_LT_BLUE << 4) | VCOL_YELLOW);

	// Draw 4x4 pixel square for each grid field

	for(char y=0; y<31; y++)
	{
		for(char x=0; x<31; x++)
		{
			char	c = gridstate[y][x];

			if (!(c & GS_HIDDEN))
			{
				if (c & GS_UNIT)
					window_draw_quad_u(x, 2 * y + (x & 1));
				else
				{
					if ((c & GS_TERRAINX) <= GTERRAIN_WATER)
						c = 1;
					else
						c = 2;

					window_draw_quad(x, 2 * y + (x & 1), c);
				}
			}
		}
	}

	int sx = winX * 8 + 24 + 4 * ox, sy = winY * 8 + 50 + 4 * oy;

	// Show the area select sprite

	spr_set(1, true, sx, sy, 48 + 14, VCOL_WHITE, false, true, true);

	sbyte tx = ox, ty = oy;

	while (!game_check_continue())
	{	
		// Move area with cursor

		tx += joyx[0]; 
		ty += joyy[0];

		// Move area with keyboard

		switch (keyb_codes[keyb_key & 0x7f])
		{
		case KEY_CSR_DOWN:
		case 's':
			sidfx_play_step();
			ty += 4;
			break;
		case KEY_CSR_UP:
		case 'w':
			sidfx_play_step();
			ty -= 4;
			break;
		case KEY_CSR_LEFT:
		case 'a':
			sidfx_play_step();
			tx -= 4;
			break;
		case KEY_CSR_RIGHT:
		case 'd':
			sidfx_play_step();
			tx += 4;
			break;
		}

		// Clip area to map

		if (tx < 0)
			tx = 0;
		else if (tx > 18)
			tx = 18;

		if (ty < 0)
			ty = 0;
		else if (ty > 24)
			ty = 24;

		sx = winX * 8 + 24 + 4 * tx;
		sy = winY * 8 + 50 + 4 * ty;

		spr_move(1, sx, sy);
		vic_waitFrame();
	}

	spr_show(1, false);

	// Close window

	window_close();

	// Draw playfield at new location

	if (ox != tx || oy != ty)
	{
		ox = tx;
		oy = ty;
		ox &= ~1;

		grid_redraw_all();

		if (gridX < ox + 2)
			gridX = ox + 2;
		else if (gridX > ox + 10)
			gridX = ox + 10;

		if (gridY < oy + 2)
			gridY = oy + 2;
		else if (gridY > oy + 6)
			gridY = oy + 6;

		cursor_fromgrid();
	}

	cursor_show();
}

// game level completed

void game_victory(void)
{
	music_patch_voice3(true);

	music_init(TUNE_RESULT);

	GamePhase = GP_VICTORY;

	char	buffer[50];
	strcpy(buffer, "VICTORY!\nYOU WIN\n\nPASSCODE\n");
	if (game_level + 1 < NUM_LEVELS)
		strcat(buffer, LevelInfos[game_level + 1].passcode);

	tovl_show(buffer, TeamColors[0]);
	tovl_color(3, VCOL_MED_GREY);
	tovl_color(4, VCOL_WHITE);
}

// game level failed

void game_defeat(void)
{
	music_patch_voice3(true);

	music_init(TUNE_RESULT);

	GamePhase = GP_LOST;

	tovl_show("DEFEAT!\nYOU LOST\n RETRY?", TeamColors[1]);
}

void game_title(void)
{
	GamePhase = GP_TITLE;
}

static const char MenuText[] = "CONTINUE\nHINT\nMUSIC\nRESIGN\nTITLE";

static const char MenuColors[] = {
	VCOL_ORANGE, VCOL_YELLOW, VCOL_WHITE, VCOL_YELLOW, 
	VCOL_ORANGE, VCOL_RED, VCOL_BLACK, VCOL_ORANGE
};

enum GameMenuAction
{
	GSA_CONTINUE,
	GSA_RESIGN,
	GSA_TITLE,
	GSA_HINT,
};

void game_hint(void)
{
	tovl_wait();

	tovl_show(LevelInfos[game_level].hint, VCOL_LT_GREY);
	tovl_wait();

	while (!game_check_continue())
		vic_waitFrame();

	tovl_hide();
	tovl_wait();

	cursor_show();
}

// show ingame menu

void game_menu(void)
{
	tovl_show(MenuText, VCOL_ORANGE);

	char	menu = 0;
	char	ma = 0;
	GameMenuAction	action = GSA_CONTINUE;

	while (true)
	{
		if (game_check_continue())
		{
			// Activate menu entry

			if (menu == 0)
				break;	
			else if (menu == 1)
			{
				action = GSA_HINT;
				break;
			}			
			else if (menu == 2)		
			{
				music_toggle();
				break;
			}
			else if (menu == 3)
			{
				action = GSA_RESIGN;
				break;
			}
			else if (menu == 4)
			{
				action = GSA_TITLE;
				break;
			}
		}

		// Move menu selection by key

		switch (keyb_codes[keyb_key & 0x7f])
		{
		case KEY_CSR_DOWN:
			if (menu < 4)
			{
				tovl_color(menu, VCOL_ORANGE);
				menu++;
			}
			break;
		case KEY_CSR_UP:
			if (menu > 0)
			{
				tovl_color(menu, VCOL_ORANGE);
				menu--;
			}
			break;
		}

		// Move menu selection by joystick

		if (joyy[0] > 0)
		{
			if (!joyBlockMove && menu < 4)
			{
				joyBlockMove = true;
				tovl_color(menu, VCOL_ORANGE);
				menu++;
			}
		}
		else if (joyy[0] < 0)
		{
			if (!joyBlockMove && menu > 0)
			{
				joyBlockMove = true;
				tovl_color(menu, VCOL_ORANGE);
				menu--;
			}
		}
		else
			joyBlockMove = false;

		tovl_color(menu, MenuColors[ma & 7]);
		ma++;

		vic_waitFrame();
	}

	tovl_hide();
	tovl_wait();

	cursor_show();

	// perform action based on menu selection

	switch (action)
	{
		case GSA_CONTINUE:
			break;

		case GSA_HINT:
			game_hint();
			break;

		case GSA_RESIGN:
			game_defeat();
			break;

		case GSA_TITLE:
			game_title();
			break;

	}
}

// Process player input

void game_input(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	// Keeping the random generator in flow
	rand();

	// Poll joystick

	joy_poll(0);

	JoystickMenu menu = JM_SELECT;

	// Check if button was down

	if (joybcount)
	{
		// Select joystick menu command

		if (joyy[0])
		{
			if (joyy[0] < 0)
			{
				if (joyx[0] < 0)
					menu = JM_REPAIR;
				else if (joyx[0] > 0)
					menu = JM_UNDO;
				else
					menu = JM_INFO;
			}
			else
			{
				if (joyx[0] < 0)
					menu = JM_MAP;
				else if (joyx[0] > 0)
					menu = JM_DONE;
				else
					menu = JM_MENU;
			}

			spr_image(0, 71 + menu);
			spr_show(0, true);
			spr_show(2, true);
		}
		else
		{
			spr_show(0, false);
			spr_show(2, false);
		}
	}

	if (joyb[0])
	{
		joybcount++;
	}
	else if (joybcount)
	{
		// Joystick button was released

		spr_show(0, false);
		spr_show(2, false);

		joybcount = 0;

		// Check for joystick menu

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
		case JM_REPAIR:
			joyBlockMove = true;
			game_repairhex();				
			break;
		case JM_INFO:
			game_show_info();
			joyBlockMove = true;
			break;
		case JM_MENU:
			game_menu();
			joyBlockMove = true;
			break;
		case JM_MAP:
			joyBlockMove = true;
			game_show_map();
			break;
		}
	}
	else if (!joyBlockMove)
	{
		// Move cursor

		cursor_move(4 * joyx[0], 4 * joyy[0]);
	}
	else if (!(joyx[0] | joyy[0]))
	{
		// Release block on joystick movement, if joystick
		// is centered again

		joyBlockMove = false;
	}

	// Poll keyboard

	keyb_poll();

	if (keyb_key)
	{
		keyRepeatDelay = 8;

		// Check key

		switch (keyb_codes[keyb_key & 0x7f])
		{
		case ' ':
			game_selecthex();
			break;
		case KEY_CSR_DOWN:
		case 's':
			sidfx_play_step();
			cursor_move(0, 24);
			break;
		case KEY_CSR_UP:
		case 'w':
			sidfx_play_step();
			cursor_move(0, -24);
			break;
		case KEY_CSR_LEFT:
		case 'a':
			sidfx_play_step();
			cursor_move(-24, 12 - 24 * (gridX & 1));
			break;
		case KEY_CSR_RIGHT:
		case 'd':
			sidfx_play_step();
			cursor_move( 24, 12 - 24 * (gridX & 1));
			break;

		case 'i':
			game_show_info();
			break;

		case 'r':
			game_repairhex();				
			break;

		case 'x':
			game_undohex();				
			break;

		case 'z':
			game_complete_phase();
			break;

		case 'm':
			game_show_map();
			break;
#if 0
		case 'v':
			game_victory();
			break;

		case 's':
		{
			for(char i=0; i<numUnits; i++)
				units[i].type ^= UNIT_TEAM;
			drawUnits();
			break;
		}
#endif
		case KEY_ESC:
			game_menu();
			break;
		}
	}
	else if (keyRepeatDelay == 0)
	{
		// Key repeat for cursor movement

		if (key_pressed(KSCAN_CSR_RIGHT))
		{
			if (key_shift())
				cursor_move(-8, 0);
			else
				cursor_move( 8, 0);
		}
		else if (key_pressed(KSCAN_CSR_DOWN))
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

	// Show info for unit or grid item under cursor

	if ((gridstate[gridY][gridX] & (GS_UNIT | GS_HIDDEN)) == GS_UNIT)
		status_update_unit(gridunits[gridY][gridX]);
	else
		status_update_unit(SelectedUnit);
}

// main in game loop

void game_loop_playing(void)
{
	// Check if new phase starts

	if (NextPhase != MovePhase)
	{
		MovePhase = NextPhase;
		game_begin_phase();
	}

	byte	pflags = MovePhaseFlags[MovePhase];

	if (pflags & MOVPHASE_INTERACTIVE)
	{
		// Have player or ai interact

		byte 	team = pflags & MOVPHASE_TEAM;
		if (team)
			game_invoke_playerai();
		else
			game_input();
	}
	else
		game_complete_phase();
}

char game_level;

// main loop

void game_loop(void)
{
	switch (GamePhase)
	{
		case GP_TITLE:
			mainmenu_open();

			GamePhase = GP_INIT;
			break;

		case GP_INIT:
			level_setup(game_level);

			GamePhase = GP_READY;
			break;

		case GP_RESTART:
			level_restart(game_level);

			GamePhase = GP_READY;
			break;

		case GP_READY:
			MovePhase = MP_ATTACK_2;
			NextPhase = MP_MOVE_1;
			SelectedUnit = 0xff;
			NumBattlePairs = 0;
			NumPaths = 0;

			music_patch_voice3(false);

			music_queue(TUNE_THEME_NEUTRAL);

			GamePhase = GP_PLAYING;
			break;

		case GP_PLAYING:
			game_loop_playing();
			if (unit_find(UNIT_COMMAND | UNIT_TEAM_2) < 0)
				game_victory();
			else if (GameDays == 0 || unit_find(UNIT_COMMAND | UNIT_TEAM_1) < 0)
				game_defeat();
			break;

		case GP_VICTORY:
			if (game_check_continue())
			{
				tovl_hide();
				tovl_wait();
				game_level++;
				GamePhase = GP_INIT;
			}
			break;

		case GP_LOST:
			if (game_check_continue())
			{
				tovl_hide();
				tovl_wait();
				GamePhase = GP_RESTART;
			}
			break;
	}
}
