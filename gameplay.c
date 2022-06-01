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

GamePhases GamePhase;
char GameDays;

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

SIDFX	SIDFXTankMove[1] = {{
	800, 1000, 
	SID_CTRL_GATE | SID_CTRL_NOISE,
	SID_ATK_100 | SID_DKY_6,
	0xf0  | SID_DKY_204,
	-50, 0,
	10, 0,
	2
}};

SIDFX	SIDFXChopperMove[3] = {{
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

SIDFX	SIDFXInfantryMove[4] = {{
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

void sidfx_play_move(byte unit)
{
	if (unit == UNIT_INFANTRY)
		sidfx_play(2, SIDFXInfantryMove, 4);	
	else if (unit == UNIT_CHOPPER)
		sidfx_play(2, SIDFXChopperMove, 3);	
	else
		sidfx_play(2, SIDFXTankMove, 1);	
}

void game_init(void)
{
	GamePhase = GP_INIT;
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

		if (d >= 20)
		{
			buffer[n++] = '2';
			d -= 20;
		}
		else if (d >= 10)
		{
			buffer[n++] = '1';
			d -= 10;
		}
		else
			buffer[n++]	= ' ';

		buffer[n++] = d + '0';
		buffer[n++] = ' ';
		buffer[n++] = 'D';
		buffer[n++] = 'A';
		buffer[n++] = 'Y';
		buffer[n++] = 'S';
		buffer[n++] = '\n';
		buffer[n++] = 0;

		tovl_show(buffer, TeamColors[(MovePhaseFlags[MovePhase] & MOVPHASE_TEAM) ? 1 : 0]);
	}
}

void game_begin_phase(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	if (pflags & MOVPHASE_INTERACTIVE)
		game_show_overlay();

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
		grid_redraw_colors();

	char	color = TeamColors[team ? 1 : 0];

	if (pflags & MOVPHASE_INTERACTIVE)
	{		
		tovl_wait();

		if (!team)
		{
			for(char i=0; i<30; i++)
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
	if (NumBattlePairs == 0)
		return;

	if (MovePhaseFlags[MovePhase] & MOVPHASE_TEAM)
		music_init(5);
	else
		music_init(4);

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

	music_init(0);
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
				sidfx_play_move(u->type & UNIT_TYPE);

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

	if (NextPhase == 0)
		GameDays--;
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


void game_show_map(void)
{
	cursor_hide();

	window_open(4, 4, 16, 16);

	window_color_rect(0, 0, 16, 16, (VCOL_LT_BLUE << 4) | VCOL_YELLOW);

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

	spr_set(1, true, sx, sy, 48 + 14, VCOL_WHITE, false, true, true);

	sbyte tx = ox, ty = oy;

	while (!game_check_continue())
	{	
		tx += joyx[0]; 
		ty += joyy[0];

		switch (keyb_codes[keyb_key & 0x7f])
		{
		case KEY_CSR_DOWN:
			sidfx_play_step();
			ty += 4;
			break;
		case KEY_CSR_UP:
			sidfx_play_step();
			ty -= 4;
			break;
		case KEY_CSR_LEFT:
			sidfx_play_step();
			tx -= 4;
			break;
		case KEY_CSR_RIGHT:
			sidfx_play_step();
			tx += 4;
			break;
		}

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

	window_close();

	if (ox != tx || oy != ty)
	{
		ox = tx;
		oy = ty;
		ox &= ~1;

		updateColors();

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

void game_input(void)
{
	byte	pflags = MovePhaseFlags[MovePhase];

	// Keeping the random generator in flow
	rand();

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

		case 'm':
			game_show_map();
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

void game_victory(void)
{
	music_init(1);

	GamePhase = GP_VICTORY;

	tovl_show("VICTORY!\nYOU WIN\nADVANCE?\n", TeamColors[0]);
}

void game_defeat(void)
{
	music_init(1);

	GamePhase = GP_LOST;

	tovl_show("DEFEAT!\nYOU LOST\n RETRY?\n", TeamColors[1]);
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

			music_init(0);

			level_setup(0);

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
				GamePhase = GP_INIT;
			}
			break;

		case GP_LOST:
			if (game_check_continue())
			{
				tovl_hide();
				tovl_wait();
				GamePhase = GP_INIT;
			}
			break;
	}
}
