#include "gamemusic.h"

#pragma region(main, 0x0900, 0x9c00, , , {code, data, bss, heap } )
#pragma region(stack, 0x9c00, 0xa000, , , { stack } )

#pragma section( music, 0)

#pragma region( music, 0xa000, 0xc000, , , {music} )

#pragma data(music)

__export char music[] = {
	#embed 0x2000 0x88 "ArmyGame.sid" 
}

// 00 - Game theme neutral - 2:33 - 62.5 BPM
// 01 - Result - 2:18 - 62.5 BPM
// 02 - Game theme Enemy winning - 1:47 - 62.5 BPM
// 03 - Game theme Player winning - 1:32 - 62.5 BPM
// 04 - Attack (Player) - 0:15 - 62.5 BPM
// 05 - Attack (Enemy) - 0:15 - 62.5 BPM
// 06 - Flight of the valkyries - 0:32 - 75 BPM
// 07 - Title - 2:14 - 75 BPM
// 08 - Jets (maybe) - 0:38 - 62.5 BPM
// 09 - Jets (Absolutely) - 0:32 - 75 BPM
// 0A - Another level Prelude - 0:43 - 41.666... BPM

#pragma data(data)

unsigned	tune_count, tune_length;
Tune		tune_queue, tune_current;

#define TUNE_LENGTH(m, s)	(100 * (60 * (m) + (s)))
#define TUNE_BPM(bpm, s)	(unsigned)((s) * 60 / (bpm) * 100)

unsigned music_lengths[] = {
	TUNE_LENGTH(2, 33),
	TUNE_LENGTH(2, 18),
	TUNE_LENGTH(1, 47),
	TUNE_LENGTH(1, 32),

	TUNE_BPM(62.5, 1),
	TUNE_BPM(62.5, 1),

	TUNE_LENGTH(0, 32),
	TUNE_LENGTH(2, 14),
	TUNE_LENGTH(0, 38),
	TUNE_LENGTH(0, 32),
	TUNE_LENGTH(0, 43),
};

void music_init(Tune tune)
{
	tune_queue = tune_current = tune;
	tune_length = music_lengths[tune_current];
	tune_count = 0;

	__asm
	{
		lda		tune
		jsr		$a000
	}
}

void music_queue(Tune tune)
{
	tune_queue = tune;
}

void music_play(void)
{
	tune_count++;
	if (tune_count == tune_length)
	{
		if (tune_current != tune_queue)
		{
			tune_current = tune_queue;
			tune_length = music_lengths[tune_current];

			__asm
			{
				lda		tune_queue
				jsr		$a000
			}
		}

		tune_count = 0;
	}

	__asm
	{
		jsr		$a003
	}
}

void music_patch_voice3(bool enable)
{
	*(char *)0xa14e = enable ? 0x20 : 0x4c;
}

