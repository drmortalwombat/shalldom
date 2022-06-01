#include "gamemusic.h"

#pragma region(main, 0x0a00, 0xa000, , , {code, data, bss, heap, stack} )

#pragma section( music, 0)

#pragma region( music, 0xa000, 0xc000, , , {music} )

#pragma data(music)

__export char music[] = {
	#embed 0x2000 0x88 "ArmyGame.sid" 
}

// 00 - Game theme neutral
// 01 - Result
// 02 - Game theme Enemy winning
// 03 - Game theme Player winning
// 04 - Attack (Player)
// 05 - Attack (Enemy)

#pragma data(data)

void music_init(char tune)
{
	__asm
	{
		lda		tune
		jsr		$a000
	}
}

void music_play(void)
{
	__asm
	{
		jsr		$a003
	}
}

void music_patch_voice3(bool enable)
{
	*(char *)0xa094 = enable ? 0x20 : 0x4c;
}

