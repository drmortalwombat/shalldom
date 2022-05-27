#include "levels.h"
#include "units.h"
#include "status.h"
#include "terrain.h"
#include "playerai.h"
#include <c64/vic.h>

void level_setup0(void)
{
	AITasks	=	aitasks;

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

	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 18, 11, 0);
	unit_add(UNIT_HEAVY_TANK | UNIT_TEAM_2, 17, 13, 1);

	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 13, 19, 0 + 8 * 3);
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2,  8, 10, 1 + 8 * 1);	
	unit_add(UNIT_LIGHT_TANK | UNIT_TEAM_2, 22, 14, 2 + 8 * 2);

	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 14, 0);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 29, 15, 1);
	unit_add(UNIT_INFANTRY | UNIT_TEAM_2, 17, 10, 2);
}

void level_setup1(void)
{
	
}

void level_setup(char level)
{
	grid_blank();

	drawBaseGrid();
	status_init();

	tovl_show("PREPARE\nTERRAIN\nDATA\n", VCOL_YELLOW);

	switch (level)
	{
	case 0:
		level_setup0();
		break;

	case 1:
		level_setup1();
		break;
	}

	tovl_hide();
	tovl_wait();

	cursor_init(3, 15);

	drawUnits();
	resetFlags();
	calcVisibility(UNIT_TEAM_1);
	calcThreatened(UNIT_TEAM_2);

	updateColors();
}
