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

MovePhases MovePhase, NextPhase;

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

SIDFX	SIDFXDing[1] = {{
	6400, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x20  | SID_DKY_204,
	0, 0,
	1, 10,
	1
}};

SIDFX	SIDFXDing2[1] = {{
	3200, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x40  | SID_DKY_204,
	0, 0,
	2, 10,
	1
}};

SIDFX	SIDFXFail[1] = {{
	800, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x50  | SID_DKY_300,
	0, 0,
	2, 10,
	1
}};

SIDFX	SIDFXUndo[2] = {{
	3200, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x40  | SID_DKY_204,
	200, 0,
	8, 0,
	1
},{
	3200, 2048, 
	SID_CTRL_GATE | SID_CTRL_RECT,
	SID_ATK_2 | SID_DKY_6,
	0x40  | SID_DKY_204,
	-200, 0,
	2, 10,
	1
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

void game_init(void)
{
	GamePhase = GP_INIT;

}

static const char * PhaseOverlay[] = 
{
	" PLAYER\n ATTACK\n\n",
	" ENEMY\n MOVES\n\n",
	nullptr,
	nullptr,
	" ENEMY\nATTACKS\n\n",
	" PLAYER\n MOVE\n\n",
	nullptr,
	nullptr,
};

void game_invoke_playerai(void);

void game_begin_phase(void)
{
	for(char i=0; i<numUnits; i++)
	{
		units[i].tx = units[i].mx;
		units[i].ty = units[i].my;
		units[i].type &= ~UNIT_COMMANDED;
		ghostUnit(i);
	}

	resetFlags();

	byte	pflags = MovePhaseFlags[MovePhase];
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
		grid_redraw_colors();

	char	color = TeamColors[team ? 1 : 0];

	if (pflags & MOVPHASE_INTERACTIVE)
	{		
		tovl_show(PhaseOverlay[MovePhase], TeamColors[team ? 1 : 0]);
		tovl_wait();

		if (!team)
		{
			for(char i=0; i<15; i++)
				vic_waitFrame();

			tovl_hide();
			tovl_wait();
			cursor_show();		
		}

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

void game_execute_battles(void)
{
	Battle	b;

	for(byte bi=0; bi<NumBattlePairs; bi++)
	{
		byte	du = BattlePairs[bi].to;
		if (du != 0xff)
		{
			hex_scroll_into_view(du);
	
			char ux = units[du].mx - ox;

			if (ux < 7)
				window_open(24, 4, 12, 15);
			else
				window_open(4, 4, 12, 15);

			units[du].flags &= ~UNIT_FLAG_REPAIR;

			battle_init(&b, du);

			for(char phase=0; phase<=16; phase++)
			{
				battle_enter_units(&b, CBT_DEFENDER, phase);
				vic_waitFrame();
			}

			for(byte bj=bi; bj<NumBattlePairs; bj++)
			{
				if (du == BattlePairs[bj].to)
				{
					BattlePairs[bj].to = 0xff;

					if (battle_num_units(&b, CBT_DEFENDER))
					{
						battle_begin_attack(&b, BattlePairs[bj].from);

						for(char phase=0; phase<=16; phase++)
						{
							battle_enter_units(&b, CBT_ATTACKER, phase);
							vic_waitFrame();
						}

						while (battle_fire(&b))
						{
							for(char phase=0; phase<8; phase++)
							{
								battle_fire_animate(&b, phase);
								vic_waitFrame();
							}
						}

						for(char i=0; i<4; i++)
						{
							for(char phase=0; phase<8; phase++)
							{
								battle_fire_animate(&b, phase);
								vic_waitFrame();
							}				
						}

						battle_complete_attack(&b);
					}
				}
			}

			battle_complete(&b);

			window_fill(0x00);
			window_close();

			drawUnits();
		}
	}

	unit_compact();
	drawUnits();
}

void game_execute_repair(void)
{
	for(char i=0; i<numUnits; i++)
	{
		if (units[i].flags & UNIT_FLAG_REPAIR)
		{
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

void game_execute_moves(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	for(char i=0; i<numUnits; i++)
	{
		if ((units[i].type & UNIT_TEAM ) == team)
			units[i].flags |= UNIT_FLAG_RESTED;
	}

	for(char pi=0; pi<NumPaths; pi++)
	{
		Path	*	p = Paths + pi;
		char		x = p->sx, y = p->sy;

		if (gridstate[y][x] & GS_UNIT)
		{
			char		ui = gridunits[y][x];

			if (!(gridstate[y][x] & GS_HIDDEN))
				hex_scroll_into_view(ui);

			Unit	*	u = units + ui;

			u->flags &= ~UNIT_FLAG_RESTED;

			char color = TeamColors[(u->type & UNIT_TEAM) ? 1 : 0];
			char image = 48 + (u->type & UNIT_TYPE);

			spr_set(3, !(gridstate[y][x] & GS_HIDDEN), 0, 0, image, color, true, false, false);

			hideUnit(ui);
			updateGridCell(x, y);

			char pi = 8;
			while (p->steps[pi - 1] == 0xff)
				pi--;

			do  {
				pi--;
				byte	state = gridstate[y][x];

				int	ux = (x - ox) * 24 + 28, uy = (y - oy) * 24 + 50 + 12 * (x & 1);
				char	d = p->steps[pi];
				y = (2 * y + (x & 1) - PathY[d]) >> 1;
				x -= PathX[d];
				int	tx = (x - ox) * 24 + 28, ty = (y - oy) * 24 + 50 + 12 * (x & 1);

				state &= gridstate[y][x];

				spr_show(3, !(state & GS_HIDDEN));

				for(char j=0; j<16; j++)
				{
					spr_move(3, ux + ((tx - ux) * j >> 4), uy + ((ty - uy) * j >> 4));
					vic_waitFrame();
				}
			} while (pi > 0);

			moveUnit(ui, x, y);
			updateGridCell(x, y);

			spr_show(3, false);
		}
	}
}

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

	NextPhase = (MovePhase + 1) & 7;
}

void game_invoke_playerai(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte	team = pflags & MOVPHASE_TEAM;

	playerai_advance(team);

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
		sidfx_play_fail();

		resetMovement();
		updateBaseGrid();
		SelectedUnit = 0xff;
	}
}

void game_repairhex(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	if (gridstate[gridY][gridX] & GS_UNIT)
	{
		char ui = gridunits[gridY][gridX];
		if ((units[ui].type & (UNIT_TEAM | UNIT_COMMANDED)) == team)
		{
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

void game_undohex(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];
	byte 	team = pflags & MOVPHASE_TEAM;

	if (gridstate[gridY][gridX] & GS_UNIT)
	{
		char ui = gridunits[gridY][gridX];
		if ((units[ui].type & (UNIT_TEAM | UNIT_COMMANDED)) == (team | UNIT_COMMANDED))
		{
			sidfx_play_undo();

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

void game_input(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	joy_poll(0);

	JoystickMenu menu = JM_SELECT;

	if (joybcount)
	{
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
		case JM_REPAIR:
			joyBlockMove = true;
			game_repairhex();				
			break;
		case JM_INFO:
		case JM_MENU:
		case JM_MAP:
			joyBlockMove = true;
			break;
		}
	}
	else if (!joyBlockMove)
	{
		cursor_move(4 * joyx[0], 4 * joyy[0])
	}
	else if (!(joyx[0] | joyy[0]))
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
			sidfx_play_step();
			cursor_move(0, 24);
			break;
		case KEY_CSR_UP:
			sidfx_play_step();
			cursor_move(0, -24);
			break;
		case KEY_CSR_LEFT:
			sidfx_play_step();
			cursor_move(-24, 12 - 24 * (gridX & 1));
			break;
		case KEY_CSR_RIGHT:
			sidfx_play_step();
			cursor_move( 24, 12 - 24 * (gridX & 1));
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

		case 'w':
			window_open(4, 4, 13, 8);
			window_write(0, 0, "HELLO WORLD");
			window_write(0, 1, "2ND LINE");
			break;

		case 'a':
			game_invoke_playerai();
			break;

		case 'e':
			window_close();
			break;
		}
	}
	else if (keyRepeatDelay == 0)
	{
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

	if ((gridstate[gridY][gridX] & (GS_UNIT | GS_HIDDEN)) == GS_UNIT)
		status_update_unit(gridunits[gridY][gridX]);
	else
		status_update_unit(SelectedUnit);
}

void game_loop_playing(void)
{
	if (NextPhase != MovePhase)
	{
		MovePhase = NextPhase;
		game_begin_phase();
	}

	byte	pflags = MovePhaseFlags[MovePhase];

	if (pflags & MOVPHASE_INTERACTIVE)
	{
		byte 	team = pflags & MOVPHASE_TEAM;
		if (team)
			game_invoke_playerai();
		else
			game_input();
	}
	else
		game_complete_phase();
}

void game_loop(void)
{
	switch (GamePhase)
	{
		case GP_INIT:

			MovePhase = MP_ATTACK_2;
			NextPhase = MP_MOVE_1;
			SelectedUnit = 0xff;
			GamePhase = GP_PLAYING;

			break;

		case GP_PLAYING:
			game_loop_playing();
			break;

		case GP_VICTORY:
			break;

		case GP_LOST:
			break;
	}
}
