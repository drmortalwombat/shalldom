#include "levels.h"
#include "units.h"
#include "status.h"
#include "terrain.h"
#include "playerai.h"
#include <c64/vic.h>

AITask	aitasks1[] = {
	{AIS_IDLE + 0 * 8,  0,  0, 0},	// 0 
	{AIS_RUSH + 0 * 8,  3, 15, 0},	// 1 -> 0
	{AIS_RUSH + 7 * 8, 17,  9, 0},  // 2 -> 7
	{AIS_RUSH + 1 * 8, 11, 22, 0},  // 3 -> 1

	{AIS_IDLE + 5 * 8,  0,  0, 6},    // 4 -> 5
	{AIS_STROLL + 6 * 8, 20, 12, 0},  // 5 -> 6
	{AIS_STROLL + 0 * 8, 29, 16, 0},  // 6
	{AIS_STROLL + 0 * 8,  3, 15, 0},  // 7 -> 0
};

#define LEVEL_SEED(x) 							(x & 0xff), (x >> 8)
#define LEVEL_DAYS(d)							d
#define LEVEL_ROAD(x, y, d, s, t) 				x, y, d, (s | (t << 5))
#define LEVEL_ROAD_END							0xff
#define LEVEL_AI_TASK(com, next, x, y, day)		(com | (next << 3)), x, y, day
#define LEVAL_AI_END							0xff
#define LEVEL_UNIT(type, x, y, ai)				(type | (ai << 4)), x, y
#define LEVEL_UNIT_END							0xff


void level_setup_cmd(const char * cmd)
{
	terrain_build(cmd[0] + (cmd[1] << 8));
	GameDays = cmd[2];
	cmd += 3;

	while (cmd[0] != LEVEL_ROAD_END)
	{
		terrain_pave_road(cmd[0], cmd[1], PathX[cmd[2]], PathY[cmd[2]], cmd[3] & 0x1f);
		cmd += 4;
	}
	cmd++;

	char	ntype[16];

	for(char i=0; i<16; i++)
		ntype[i] = 0;

	char	team = UNIT_TEAM_1;
	while (true)
	{
		char	unit = cmd[0] & 0x0f;
		bool	next = false;

		if (unit == UNIT_COMMAND_END)
		{
			unit = UNIT_COMMAND;
			next = true;
		}

		unit_add(unit | team, cmd[1], cmd[2], (cmd[0] >> 4 << 3) | ntype[unit]);
		cmd += 3;
		ntype[unit]++;

		if (next)
		{
			if (team == UNIT_TEAM_1)
			{
				for(char i=0; i<16; i++)
					ntype[i] = 0;
				team = UNIT_TEAM_2;
			}
			else
				break;
		}
	}

	AITasks	= (AITask *)cmd;
}

static const char level_data1[] = {
	LEVEL_SEED(23893),
	LEVEL_DAYS(14),

	LEVEL_ROAD( 4, 16,  1, 13, GTERRAIN_ROAD),
	LEVEL_ROAD(17,  9,  2,  1, GTERRAIN_ROAD),
	LEVEL_ROAD(18, 10,  3,  1, GTERRAIN_ROAD),
	LEVEL_ROAD(18, 11,  2,  3, GTERRAIN_ROAD),
	LEVEL_ROAD(21, 12,  1,  5, GTERRAIN_ROAD),
	LEVEL_ROAD_END,


	LEVEL_UNIT(UNIT_HEAVY_TANK,  4, 15, 0),
	LEVEL_UNIT(UNIT_HEAVY_TANK,  4, 16, 0),

	LEVEL_UNIT(UNIT_LIGHT_TANK,  5, 14, 0),
	LEVEL_UNIT(UNIT_LIGHT_TANK,  5, 15, 0),
	LEVEL_UNIT(UNIT_LIGHT_TANK,  5, 16, 0),

	LEVEL_UNIT(UNIT_INFANTRY,  6, 13, 0),
	LEVEL_UNIT(UNIT_INFANTRY,  6, 14, 0),
	LEVEL_UNIT(UNIT_INFANTRY,  7, 13, 0),
	LEVEL_UNIT(UNIT_COMMAND_END,  3, 15, 0),

	LEVEL_UNIT(UNIT_HEAVY_TANK, 18, 11, 4),
	LEVEL_UNIT(UNIT_HEAVY_TANK, 17, 13, 4),

	LEVEL_UNIT(UNIT_LIGHT_TANK, 13, 19, 3),
	LEVEL_UNIT(UNIT_LIGHT_TANK,  8, 10, 1),	
	LEVEL_UNIT(UNIT_LIGHT_TANK, 22, 14, 2),

	LEVEL_UNIT(UNIT_INFANTRY, 15,  9, 0),
	LEVEL_UNIT(UNIT_INFANTRY, 29, 15, 0),
	LEVEL_UNIT(UNIT_INFANTRY, 17, 10, 0),

	LEVEL_UNIT(UNIT_COMMAND_END, 29, 16, 0),

	LEVEL_AI_TASK(AIS_IDLE,   0,  0,  0, 0), // 0 
	LEVEL_AI_TASK(AIS_RUSH,   0,  3, 15, 0), // 1 -> 0
	LEVEL_AI_TASK(AIS_RUSH,   7, 17,  9, 0), // 2 -> 7
	LEVEL_AI_TASK(AIS_RUSH,   1, 11, 22, 0), // 3 -> 1

	LEVEL_AI_TASK(AIS_IDLE,   5,  0,  0, 6), // 4 -> 5
	LEVEL_AI_TASK(AIS_STROLL, 6, 20, 12, 0), // 5 -> 6
	LEVEL_AI_TASK(AIS_STROLL, 0, 29, 16, 0), // 6
	LEVEL_AI_TASK(AIS_STROLL, 0,  3, 15, 0), // 7 -> 0
};

void level_setup1(void)
{
#if 1
	level_setup_cmd(level_data1);
#else
	AITasks	=	aitasks1;

	terrain_build(23893);
	terrain_pave_road( 4, 16,  1, -1, 13);
	terrain_pave_road(17,  9,  1,  1,  1);
	terrain_pave_road(18, 10,  0,  2,  1);
	terrain_pave_road(18, 11,  1,  1,  3);
	terrain_pave_road(21, 12,  1, -1,  5);

	unit_add(UNIT_COMMAND | UNIT_TEAM_1,  3, 15, 0);
	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 29, 16, 0);

	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1,  4, 15, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1,  4, 16, 1);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  5, 14, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  5, 15, 1);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  5, 16, 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  6, 13, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  6, 14, 1);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  7, 13, 2);

	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 18, 11, 0 + 8 * 4);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 17, 13, 1 + 8 * 4);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 13, 19, 0 + 8 * 3);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2,  8, 10, 1 + 8 * 1);	
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 22, 14, 2 + 8 * 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 15,  9, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 15, 1);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 17, 10, 2);

	GameDays = 14;
#endif
}

AITask	aitasks0[] = {
	{AIS_IDLE   + 0 * 8,  0,  0, 0},	// 0
	{AIS_RUSH   + 0 * 8, 13, 14, 0},	// 1 -> 0
	{AIS_STROLL + 3 * 8, 23, 12, 0},	// 2 -> 3
	{AIS_HOLD   + 0 * 8, 23, 12, 0},	// 3
	{AIS_HOLD   + 1 * 8, 19, 13, 6}		// 4 -> 1
};

static const char level_data0[] = {
	LEVEL_SEED(36451),
	LEVEL_DAYS(8),

	LEVEL_ROAD_END,
	
	LEVEL_UNIT(UNIT_LIGHT_TANK, 14, 13, 0),
	LEVEL_UNIT(UNIT_LIGHT_TANK, 14, 14, 0),
	LEVEL_UNIT(UNIT_LIGHT_TANK, 15, 13, 0),

	LEVEL_UNIT(UNIT_INFANTRY, 14, 12, 0),
	LEVEL_UNIT(UNIT_INFANTRY, 15, 11, 0),

	LEVEL_UNIT(UNIT_COMMAND_END, 13, 11, 0),

	LEVEL_UNIT(UNIT_LIGHT_TANK, 18, 10, 1),
	LEVEL_UNIT(UNIT_LIGHT_TANK, 18, 14, 4),
	LEVEL_UNIT(UNIT_LIGHT_TANK, 16, 16, 2),

	LEVEL_UNIT(UNIT_INFANTRY, 22, 12, 0),
	LEVEL_UNIT(UNIT_INFANTRY, 24, 11, 0),

	LEVEL_UNIT(UNIT_COMMAND_END, 23, 11, 0),

	LEVEL_AI_TASK(AIS_IDLE,   0,  0,  0, 0),	// 0
	LEVEL_AI_TASK(AIS_RUSH,   0, 13, 14, 0),	// 1 -> 0
	LEVEL_AI_TASK(AIS_STROLL, 3, 23, 12, 0),	// 2 -> 3
	LEVEL_AI_TASK(AIS_HOLD,   0, 23, 12, 0),	// 3
	LEVEL_AI_TASK(AIS_HOLD,   1, 19, 13, 6)		// 4 -> 1
};

void level_setup0(void)
{
#if 1
	level_setup_cmd(level_data0);
#else

	AITasks	= aitasks0;

	terrain_build(36451);
	

	LEVEL_UNIT(UNIT_LIGHT_TANK, 14, 13, 0);
	LEVEL_UNIT(UNIT_LIGHT_TANK, 14, 14, 0);
	LEVEL_UNIT(UNIT_LIGHT_TANK, 15, 13, 0);

	LEVEL_UNIT(UNIT_INFANTRY, 14, 12, 0);
	LEVEL_UNIT(UNIT_INFANTRY, 15, 11, 0);

	LEVEL_UNIT(UNIT_COMMAND_END, 13, 11, 0);

	LEVEL_UNIT(UNIT_LIGHT_TANK, 18, 10, 1);
	LEVEL_UNIT(UNIT_LIGHT_TANK, 18, 14, 4);
	LEVEL_UNIT(UNIT_LIGHT_TANK, 16, 16, 2);

	LEVEL_UNIT(UNIT_INFANTRY, 22, 12, 0);
	LEVEL_UNIT(UNIT_INFANTRY, 24, 11, 0);

	LEVEL_UNIT(UNIT_COMMAND_END, 23, 11, 0);

	GameDays = 8;
#endif
}

AITask	aitasks2[] = {
	{AIS_IDLE   + 0 * 8,  0,  0, 0},	// 0
	{AIS_RUSH   + 0 * 8,  6,  8, 0},    // 1
	{AIS_RUSH   + 0 * 8,  2, 18, 0},    // 2
};

static const char level_data2[] = {
	LEVEL_SEED(9756),
	LEVEL_DAYS(16),

	LEVEL_ROAD( 7, 14,  1,  7, GTERRAIN_ROAD),
	LEVEL_ROAD(14, 12,  1,  1, GTERRAIN_ROAD),
	LEVEL_ROAD(21, 11,  1,  1, GTERRAIN_ROAD),
	LEVEL_ROAD_END,
	
	LEVEL_UNIT(UNIT_SCOUT_DRONE, 5, 13, 0),
	LEVEL_UNIT(UNIT_SCOUT_DRONE, 5, 14, 0),

	LEVEL_UNIT(UNIT_LIGHT_TANK,  8, 13, 0),
	LEVEL_UNIT(UNIT_LIGHT_TANK,  8, 14, 0),
	LEVEL_UNIT(UNIT_LIGHT_TANK,  8, 15, 0),

	LEVEL_UNIT(UNIT_HEAVY_TANK, 7, 13, 0),
	LEVEL_UNIT(UNIT_HEAVY_TANK, 7, 14, 0),

	LEVEL_UNIT(UNIT_ARTILLERY, 7, 11, 0),
	LEVEL_UNIT(UNIT_ARTILLERY, 8, 11, 0),

	LEVEL_UNIT(UNIT_INFANTRY,  7, 12, 0),
	LEVEL_UNIT(UNIT_INFANTRY,  6, 15, 0),

	LEVEL_UNIT(UNIT_COMMAND_END, 3, 13, 0),

	LEVEL_UNIT(UNIT_ARTILLERY, 19, 11, 3),
	LEVEL_UNIT(UNIT_ARTILLERY, 19, 14, 3),

	LEVEL_UNIT(UNIT_INFANTRY, 15, 12, 0),
	LEVEL_UNIT(UNIT_INFANTRY, 15, 15, 0),

	LEVEL_UNIT(UNIT_LIGHT_TANK, 16, 11, 5),
	LEVEL_UNIT(UNIT_LIGHT_TANK, 16, 12, 5),
	LEVEL_UNIT(UNIT_LIGHT_TANK, 17, 11, 5),

	LEVEL_UNIT(UNIT_HEAVY_TANK, 18, 12, 0),
	LEVEL_UNIT(UNIT_HEAVY_TANK, 18, 13, 0),

	LEVEL_UNIT(UNIT_SCOUT_DRONE, 20,  9, 1),
	LEVEL_UNIT(UNIT_SCOUT_DRONE, 17, 18, 2),

	LEVEL_UNIT(UNIT_COMMAND_END, 28, 17, 0),

	LEVEL_AI_TASK(AIS_IDLE,   0,  0,  0, 0),	// 0
	LEVEL_AI_TASK(AIS_RUSH,   0,  6,  8, 0),	// 1 -> 0
	LEVEL_AI_TASK(AIS_STROLL, 0,  2, 18, 0),	// 2 -> 0
	LEVEL_AI_TASK(AIS_IDLE,   0,  0,  0, 8),	// 3 -> 4	
	LEVEL_AI_TASK(AIS_RUSH,   0, 16, 14, 0),	// 4 -> 0
	LEVEL_AI_TASK(AIS_RUSH,   0,  9, 10, 0),	// 5 -> 6
	LEVEL_AI_TASK(AIS_STROLL, 0,  3, 12, 0),	// 6 -> 0
};

void level_setup2(void)
{
	AITasks	= aitasks2;

	terrain_build(9756);
	terrain_pave_road(  7, 14,  1, -1, 7);
	terrain_pave_road( 14, 12,  1, -1, 1);
	terrain_pave_road( 21, 11,  1, -1, 1);
	
	unit_add(UNIT_COMMAND | UNIT_TEAM_1, 3,  13, 0);
	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 28, 17, 0);

	unit_add(UNIT_SCOUT_DRONE | UNIT_TEAM_1, 5, 13, 0);
	unit_add(UNIT_SCOUT_DRONE | UNIT_TEAM_1, 5, 14, 1);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  8, 13, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  8, 14, 1);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1,  8, 15, 2);

	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1, 7, 13, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1, 7, 14, 1);

	unit_add(UNIT_ARTILLERY | UNIT_TEAM_1, 7, 11, 0);
	unit_add(UNIT_ARTILLERY | UNIT_TEAM_1, 8, 11, 1);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  7, 12, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  6, 15, 1);


	unit_add(UNIT_ARTILLERY | UNIT_TEAM_2, 19, 11, 0);
	unit_add(UNIT_ARTILLERY | UNIT_TEAM_2, 19, 14, 1);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 15, 12, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 15, 15, 1);

	unit_add(UNIT_SCOUT_DRONE | UNIT_TEAM_2, 20,  9, 0 + 8 * 1);
	unit_add(UNIT_SCOUT_DRONE | UNIT_TEAM_2, 17, 18, 1 + 8 * 2);

	GameDays = 20;
}

void level_setup3(void)
{
	AITasks	= aitasks2;

	terrain_build(26717);
	
	unit_add(UNIT_COMMAND | UNIT_TEAM_1, 3,  13, 0);
	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 28, 17, 0);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 21, 15, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 21, 16, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 22, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 22, 15, 0);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 23, 13, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 23, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 24, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 24, 15, 0);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 25, 13, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 25, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 26, 13, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 26, 14, 0);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 27, 12, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 27, 13, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 28, 12, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 28, 13, 0);

	GameDays = 20;
}

void level_setup(char level)
{
	grid_blank();

	drawBaseGrid();
	status_init();

	tovl_show("PREPARE\nTERRAIN\nDATA", VCOL_YELLOW);

	numUnits = 0;

	switch (level)
	{
	case 0:
		level_setup_cmd(level_data0);
		break;

	case 1:
		level_setup_cmd(level_data1);
		break;

	case 2:
		level_setup_cmd(level_data2);
		break;

	case 3:
		level_setup3();
		break;
	}

	tovl_hide();

	grid_blank();
	grid_init_sprite();

	drawBaseGrid();

	tovl_wait();

	char	ci = unit_find(UNIT_COMMAND | UNIT_TEAM_1);

	cursor_init(units[ci].mx, units[ci].my);

	drawUnits();
	resetFlags();
	calcVisibility(UNIT_TEAM_1);
	calcThreatened(UNIT_TEAM_2);

	updateColors();
}
