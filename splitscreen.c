#include "splitscreen.h"
#include "hexdisplay.h"
#include <c64/vic.h>
#include <audio/sidfx.h>

RIRQCode	rirqtop, rirqbottom, rirqsidfx;
RIRQCode	rirqoverlay[3];

char sprovlx[3];
char sprovlimg[3][8];

#define xps		24 + 56

static const int sprvolpos[42] = {
	320 + 24,
	320 + 24,
	320 + 24,
	320 + 24,
	xps - 16 * 16, xps - 15 * 15, xps - 14 * 14, xps - 13 * 13,
	xps - 12 * 12, xps - 11 * 11, xps - 10 * 10, xps -  9 *  9,
	xps -  8 *  8, xps -  7 *  7, xps -  6 *  6, xps -  5 *  5,
	xps -  4 *  4, xps -  3 *  3, xps -  2 *  2, xps -  1 *  1,
	xps,
	xps,
	xps,
	xps,
	xps,
	xps +  1 *  1, xps +  2 *  2, xps +  3 *  3, xps +  4 *  4,
	xps +  5 *  5, xps +  6 *  6, xps +  7 *  7, xps +  8 *  8,
	xps +  9 *  9, xps + 10 * 10, xps + 11 * 11, xps + 12 * 12,
	xps + 13 * 13, xps + 14 * 14, xps + 15 * 15, xps + 16 * 16,
	320 + 24
};

char ovlline;

__interrupt void split_overlay_irq(void)
{
	char	k = ovlline;
	__assume(k < 3);

	char	y = 84 + 36 * k;
	int		x = sprvolpos[sprovlx[k]];
	char	high = 0;

//#assign i 0
//#repeat
	for(char i=0; i<8; i++)
	{
		vic.spr_pos[i].y = y;
		vic.spr_pos[i].x = x;
		if (x & 0x100)
			high |= 1 << i;
		Screen[0x3f8 + i] = sprovlimg[k][i];
		x += 26;
	}
//#assign i i + 1
//#until i == 8
//#undef i
	vic.spr_msbx = high;

	if (sprovlx[k] != 20 && sprovlx[k] != 41)
		sprovlx[k]++;
}

__interrupt void split_sidfx_irq(void)
{
	sidfx_loop_2();
	__asm {
		jsr 0xa003
	}
}

__interrupt void split_music_irq(void)
{
	__asm {
		jsr 0xa003
	}
}

void split_init(void)
{
	rirq_init(false);

	vic.ctrl1 = VIC_CTRL1_BMM | VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3;
	vic.ctrl2 = VIC_CTRL2_MCM | VIC_CTRL2_CSEL;
	
	vic.color_back = VCOL_BLACK;

	vic_setmode(VICM_HIRES_MC, Screen, Hires);

	rirq_build(&rirqtop, 4); 
	rirq_write(&rirqtop, 0, &vic.ctrl2, VIC_CTRL2_MCM | VIC_CTRL2_CSEL);
	rirq_write(&rirqtop, 1, &vic.memptr, 0x28);
	rirq_write(&rirqtop, 2, &vic.ctrl1, VIC_CTRL1_BMM | VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3);
	rirq_call(&rirqtop, 3, split_music_irq);

	rirq_build(&rirqbottom, 4); 
	rirq_delay(&rirqbottom, 10);
	rirq_write(&rirqbottom, 1, &vic.memptr, 0x24);
	rirq_write(&rirqbottom, 2, &vic.ctrl1, VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3);
	rirq_write(&rirqbottom, 3, &vic.ctrl2, VIC_CTRL2_CSEL);

	for(int i=0; i<3; i++)
	{
		rirq_build(rirqoverlay + i, 2);
		rirq_write(rirqoverlay + i, 0, &ovlline, i);
		rirq_call(rirqoverlay + i, 1, split_overlay_irq);
	}

	rirq_build(&rirqsidfx, 1);    
	rirq_call(&rirqsidfx, 0, split_sidfx_irq);

	rirq_set(0, 10,          &rirqtop);
	rirq_set(1, 49 + 8 * 24, &rirqbottom);
	rirq_set(2, 120,         &rirqsidfx);
	rirq_sort();

	rirq_start();
}


void split_overlay_show(void)
{
	rirq_wait();
	for(int i=0; i<3; i++)
		rirq_set(i + 3, 70 + 36 * i, rirqoverlay + i);
	rirq_sort();
}

void split_overlay_hide(void)
{
	rirq_wait();
	for(int i=0; i<3; i++)
		rirq_clear(i + 3);
	rirq_sort();
}
