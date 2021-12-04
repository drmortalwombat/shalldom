#include "units.h"

struct UnitInfo		UnitInfos[8] = {
	{
		0x04, 0x01,
		{127, 24, 12, 12, 24, 127},
		"INFANTRY"
	},
	{
		0x05, 0x01,
		{127, 24, 12, 6, 127, 127},
		"LIGHT TANK"
	},
	{
		0x86, 0x03,
		{8, 8, 8, 8, 8, 8},
		"AIR CAVALRY"
	},
	{
		0x04, 0x01,
		{127, 127, 48, 12, 127, 127},
		"HEAVY TANK"
	},
	{
		0x05, 0x0f,
		{127, 127, 24, 12, 6, 127},
		"AIR DEFENCE"
	},
	{
		0x88, 0x01,
		{8, 8, 8, 8, 8, 8},
		"BOMBER SQUAD"
	},
	{
		0x05, 0x02,
		{24, 24, 24, 24, 127, 127},
		"HOVERCRAFT"
	},
	{
		0x04, 0xfc,
		{127, 127, 8, 24, 8, 127},
		"ARTILLERY"
	}
};

Unit	units[32];
byte	numUnits;

