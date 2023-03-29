#include "splitscreen.h"
#include "hexdisplay.h"
#include <c64/vic.h>
#include <audio/sidfx.h>
#include "gamemusic.h"

RIRQCode	rirqtop, rirqbottom, rirqsidfx, rirqfinal;
RIRQCode	rirqoverlay[5];

char sprovlc[5];
char sprovlx[5];
char sprovlimg[5][8];

bool ntsc;

#define xps		24 + 56

// Table of sprite positions for wipe in/out

static const int sprovlpos[50] = {
	// 0
	320 + 24, 320 + 24,
	320 + 24, 320 + 24,
	320 + 24, 320 + 24,
	320 + 24, 320 + 24,

	// 8
	xps - 15 * 15, xps - 14 * 14, xps - 13 * 13,
	xps - 12 * 12, xps - 11 * 11, xps - 10 * 10, xps -  9 *  9,
	xps -  8 *  8, xps -  7 *  7, xps -  6 *  6, xps -  5 *  5,
	xps -  4 *  4, xps -  3 *  3, xps -  2 *  2, xps -  1 *  1,
	xps,
	xps +  1 *  1,	
	xps,

	// 26
	xps, xps,
	xps, xps,
	xps, xps,
	xps, xps,

	// 34
	xps +  1 *  1, xps +  2 *  2, xps +  3 *  3, xps +  4 *  4,
	xps +  5 *  5, xps +  6 *  6, xps +  7 *  7, xps +  8 *  8,
	xps +  9 *  9, xps + 10 * 10, xps + 11 * 11, xps + 12 * 12,
	xps + 13 * 13, xps + 14 * 14, xps + 15 * 15,

	// 49
	320 + 24
};

char ovlline;

__interrupt void split_overlay_irq(void)
{
	char	k = ovlline;
	__assume(k < 5);

	char	y = 64 + 36 * k;
	char	c = sprovlc[k];
	int		x = sprovlpos[sprovlx[k]];
	char	high = 0;

	// Unrolled sprite update routine
	if (x < 320 + 24)
	{
#assign i 0
#repeat
	//	for(char i=0; i<8; i++)
		{
			vic.spr_color[i] = c;
			vic.spr_pos[i].y = y;
			vic.spr_pos[i].x = x;
			if (x & 0xff00)
				high |= 1 << i;
			Screen[0x3f8 + i] = sprovlimg[k][i];
			x += 26;
		}
#assign i i + 1
#until i == 8
#undef i
		vic.spr_msbx = high;
	}

	if (sprovlx[k] != 25 && sprovlx[k] != 49)
		sprovlx[k]++;
}

__interrupt void split_sidfx_irq(void)
{
	sidfx_loop_2();
	music_play();
}

__interrupt void split_music_irq(void)
{
	music_play();
}

void split_init(void)
{
	vic_waitTop();
	vic_waitBottom();
	char	max = 0;
	while (vic.ctrl1 & VIC_CTRL1_RST8)
	{
		if (vic.raster > max)
			max = vic.raster;
	}

	ntsc = max < 8;

	rirq_init(false);

	vic.ctrl1 = VIC_CTRL1_BMM | VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3;
	vic.ctrl2 = VIC_CTRL2_MCM | VIC_CTRL2_CSEL;
	
	vic.color_back = VCOL_BLACK;

	vic_setmode(VICM_HIRES_MC, Screen, Hires);

	rirq_build(&rirqtop, 3); 
	rirq_write(&rirqtop, 0, &vic.ctrl2, VIC_CTRL2_MCM | VIC_CTRL2_CSEL);
	rirq_write(&rirqtop, 1, &vic.ctrl1, VIC_CTRL1_BMM | VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3);
	rirq_call(&rirqtop, 2, split_sidfx_irq);

	// Interrupt for status line

	rirq_build(&rirqbottom, 4); 
	rirq_delay(&rirqbottom, 10);
	rirq_write(&rirqbottom, 1, &vic.memptr, 0x24);
	rirq_write(&rirqbottom, 2, &vic.ctrl1, VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3);
	rirq_write(&rirqbottom, 3, &vic.ctrl2, VIC_CTRL2_CSEL);

	// Interrupts for text overlay

	for(int i=0; i<5; i++)
	{
		rirq_build(rirqoverlay + i, 2);
		rirq_write(rirqoverlay + i, 0, &ovlline, i);
		rirq_call(rirqoverlay + i, 1, split_overlay_irq);
	}

	// Second SID interrupt 2x speed playback

	rirq_build(&rirqsidfx, 1);    
	rirq_call(&rirqsidfx, 0, split_music_irq);

	rirq_build(&rirqfinal, 1);    
	rirq_write(&rirqfinal, 0, &vic.memptr, 0x28);


	rirq_set(0, 10,          &rirqtop);
	rirq_set(1, 49 + 8 * 24, &rirqbottom);
	rirq_set(2, 136,         &rirqsidfx);
	rirq_set(3, 250,         &rirqfinal);
	rirq_sort();

	rirq_start();
}


void split_overlay_show(char mask)
{
	// Enable overlay text interrupts

	vic_waitBottom();
	rirq_wait();
	for(int i=0; i<5; i++)
	{
		if (mask & (1 << i))
			rirq_set(i + 4, 50 + 36 * i, rirqoverlay + i);
	}
	rirq_sort();
}

void split_overlay_hide(void)
{
	// Disable overlay text interrupts

	vic_waitBottom();
	rirq_wait();
	for(int i=0; i<5; i++)
		rirq_clear(i + 4);
	rirq_sort();
}
