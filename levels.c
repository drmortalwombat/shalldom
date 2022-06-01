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

void level_setup1(void)
{
	AITasks	=	aitasks1;

	terrain_build(23893);
	terrain_pave_road( 4, 16,  1, -1, 13);
	terrain_pave_road(17,  9,  1,  1,  1);
	terrain_pave_road(18, 10,  0,  2,  1);
	terrain_pave_road(18, 11,  1,  1,  3);
	terrain_pave_road(21, 12,  1, -1,  5);
	drawBaseGrid();

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

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 15, 1);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 17, 10, 2);

	GameDays = 10;
}

AITask	aitasks0[] = {
	{AIS_IDLE   + 0 * 8,  0,  0, 0},	// 0
	{AIS_RUSH   + 0 * 8, 13, 14, 0},	// 1 -> 0
	{AIS_STROLL + 3 * 8, 23, 12, 0},	// 2 -> 3
	{AIS_HOLD   + 0 * 8, 23, 12, 0},	// 3
	{AIS_HOLD   + 1 * 8, 19, 13, 6}		// 4 -> 1
};

void level_setup0(void)
{
	AITasks	= aitasks0;

	terrain_build(36451);
	drawBaseGrid();
	
	unit_add(UNIT_COMMAND | UNIT_TEAM_1, 13, 11, 0);
	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 23, 11, 0);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1, 14, 13, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1, 14, 14, 1);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1, 15, 13, 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 14, 12, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1, 15, 11, 1);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 18, 10, 0 + 8 * 1);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 18, 14, 1 + 8 * 4);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 16, 16, 2 + 8 * 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 22, 12, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 24, 11, 1);

	GameDays = 8;
}

void level_setup2(void)
{
	terrain_build(61061);
	terrain_pave_road( 1, 8,  1, -1, 13);
	drawBaseGrid();
	
	unit_add(UNIT_COMMAND | UNIT_TEAM_1, 3, 10, 0);
	unit_add(UNIT_SCOUT_DRONE | UNIT_TEAM_1, 5, 10, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_1,  9, 10, 0);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_1, 15, 7, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_1,  1, 8, 0);
	unit_add(UNIT_CHOPPER | UNIT_TEAM_1,  3, 13, 0);
	unit_add(UNIT_AIR_DEFENCE | UNIT_TEAM_1,  8, 16, 0);
	unit_add(UNIT_BOMBER | UNIT_TEAM_1, 14, 16, 0);
	unit_add(UNIT_HOVERCRAFT | UNIT_TEAM_1, 15, 11, 0);
	unit_add(UNIT_ARTILLERY | UNIT_TEAM_1, 12, 16, 0);

	unit_add(UNIT_COMMAND | UNIT_TEAM_2, 24, 9, 0);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2,  18,  9, 0);

	GameDays = 20;
}

void level_setup(char level)
{
	grid_blank();

	drawBaseGrid();
	status_init();

	tovl_show("PREPARE\nTERRAIN\nDATA\n", VCOL_YELLOW);

	numUnits = 0;
	
	switch (level)
	{
	case 0:
		level_setup0();
		break;

	case 1:
		level_setup1();
		break;

	case 2:
		level_setup2();
		break;
	}

	tovl_hide();
	tovl_wait();

	char	ci = unit_find(UNIT_COMMAND | UNIT_TEAM_1);

	cursor_init(units[ci].mx, units[ci].my);

	drawUnits();
	resetFlags();
	calcVisibility(UNIT_TEAM_1);
	calcThreatened(UNIT_TEAM_2);

	updateColors();
}
