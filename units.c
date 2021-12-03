#include "units.h"

struct UnitInfo		UnitInfos[8] = {
	{
		0x04, 8,
		{127, 4, 2, 2, 4, 127},
		"INFANTRY"
	},
	{
		0x05, 16,
		{127, 8, 4, 2, 127, 127},
		"LIGHT TANK"
	},
	{
		0x86, 6,
		{1, 1, 1, 1, 1, 1},
		"AIR CAVALRY"
	},
	{
		0x04, 8,
		{127, 127, 8, 2, 127, 127},
		"HEAVY TANK"
	},
	{
		0x05, 10,
		{127, 127, 5, 2, 8, 127},
		"AIR DEFENCE"
	},
	{
		0x88, 8,
		{1, 1, 1, 1, 1, 1},
		"BOMBER SQUAD"
	},
	{
		0x05, 12,
		{2, 2, 2, 2, 127, 127},
		"HOVERCRAFT"
	},
	{
		0x04, 6,
		{127, 127, 6, 2, 6, 127},
		"ARTILLERY"
	}
};

Unit	units[32];
byte	numUnits;

