#include "window.h"
#include <assert.h>

const byte colorfont[] = {

#embed "colorfont.bin"

};

char		winX, winY, winW, winH;
char	*	winP;

void window_fill(char pat)
{
	char	*	wp = winP;
	char		w = 8 * winW;
	
	for(char y=0; y<winH; y++)
	{
		for(char x=0; x<w; x++)
			wp[x] = pat;

		wp += 320;
	}
}


void window_color_rect(char x, char y, char w, char h, char color)
{
	char	*	cp = Screen + 40 * (winY + y) + (winX + x);

	for(char i=0; i<h; i++)
	{
		for(char j=0; j<w; j++)
			cp[j] = color;
		cp += 40;
	}
}

void window_fill_rect(char x, char y, char w, char h, char pat)
{
	char	*	wp = winP + 8 * (40 * y + x);
	char		bw = 8 * w;
	
	for(char i=0; i<h; i++)
	{
		for(char j=0; j<bw; j++)
			wp[j] = pat;
		wp += 320;
	}
}

void window_scroll(void)
{
	char	*	wp = winP;
	char		w = 8 * winW;
	
	for(char y=1; y<winH; y++)
	{
		char	*	hp = wp + 320;
		for(char x=0; x<w; x++)
			wp[x] = hp[x];

		wp = hp;
	}
	for(char x=0; x<w; x++)
		wp[x] = 0;
}

void window_write(char x, char y, const char * text)
{
	char	*	wp = winP + 8 * (40 * y + x);


	char 	i = 0;
	char	c = text[i];
	while (c)
	{
		const char * cp = colorfont + 8 * (c & 0x3f);

		for(char j=0; j<8; j++)
			wp[j] = cp[j];

		wp += 8;
		i++;
		c = text[i];
	}
}

static char cpad[] = {0x00, 0x55, 0xaa, 0xff};

void window_draw_vbar(char x, char y, char h, char c)
{
	char	*	wp = winP + (8 * (40 * (y >> 3) + (x >> 2)) | (y & 7));

	char and = 0xc0 >> (2 * (x & 3));
	char or = cpad[c] & and;
	and = ~and;

	for(char iy=0; iy<h; iy++)
	{
		*wp = (*wp & and) | or;
		wp++;
		if (!((unsigned)wp & 7))
			wp += 312;
	}
}

void window_draw_quad(char x, char y, char c)
{
	char	*	wp = winP + (8 * (40 * (y >> 2) + (x >> 1)) | 2 * (y & 3));

	char and = 0xf0 >> (4 * (x & 1));
	char or = cpad[c] & and;
	and = ~and;

	*wp = (*wp & and) | or; wp++;
	*wp = (*wp & and) | or; wp++;
	if (!((unsigned)wp & 7))
		wp += 312;
	*wp = (*wp & and) | or; wp++;
	*wp = (*wp & and) | or; wp++;
}

void window_draw_quad_u(char x, char y)
{
	char	*	wp = winP + (8 * (40 * (y >> 2) + (x >> 1)) | 2 * (y & 3));

	char or = 0xf0 >> (4 * (x & 1));
	char and = ~or;
	or &= 0xcc;
	

	*wp = (*wp & and) | or wp++;
	*wp = (*wp & and) | or; wp++;
	if (!((unsigned)wp & 7))
		wp += 312;
	*wp = (*wp & and) | or; wp++;
	*wp = (*wp & and); wp++;
}

void window_put_sprite(char x, char y, const char * sprite)
{
	char	*	wp = winP + (8 * (40 * (y >> 3) + x) | (y & 7));

	for(char iy=0; iy<21; iy++)
	{
		char d, m;

		d = sprite[0];
		m = ((d & 0xaa) >> 1) | ((d & 0x55) << 1) | d;
		wp[0] = (wp[0] & ~m) | (d ^ ((~d & 0xaa) >> 1));

		d = sprite[1];
		m = ((d & 0xaa) >> 1) | ((d & 0x55) << 1) | d;
		wp[8] = (wp[8] & ~m) | (d ^ ((~d & 0xaa) >> 1));

		d = sprite[2];
		m = ((d & 0xaa) >> 1) | ((d & 0x55) << 1) | d;
		wp[16] = (wp[16] & ~m) | (d ^ ((~d & 0xaa) >> 1));

		sprite += 3;
		wp++;
		if (!((unsigned)wp & 7))
			wp += 312;
	}
}

void window_clear_sprite(char x, char y, char fill)
{
	char	*	wp = winP + (8 * (40 * (y >> 3) + x) | (y & 7));

	for(char iy=0; iy<21; iy++)
	{
		wp[0] = fill;
		wp[8] = fill;
		wp[16] = fill;

		wp++;
		if (!((unsigned)wp & 7))
			wp += 312;
	}	
}

void window_open(char x, char y, char w, char h)
{
	winX = x;
	winY = y;
	winW = w;
	winH = h;

	winP = Hires + 320 * winY + 8 * winX;

	char	*	wp = winP - 328;

	char	*	tp = wp;
	tp[4] |= 0x0f;
	tp[5] |= 0x0f;
	tp[6] = (tp[6] & 0xf0) | 0x0c;
	tp[7] = (tp[7] & 0xf0) | 0x0c;

	tp += 8;
	for(char i=0; i<w; i++)
	{
		tp[4] = 0xff;
		tp[5] = 0xff;
		tp[6] = 0x00;
		tp[7] = 0x00;
		tp += 8;
	}

	tp[4] |= 0xf0;
	tp[5] |= 0xf0;
	tp[6] = (tp[6] & 0x0f) | 0x30;
	tp[7] = (tp[7] & 0x0f) | 0x30;

	wp += 320;
	for(char i=0; i<h; i++)
	{
		tp = wp;

		for(char j=0; j<8; j++)
			tp[j] = (tp[j] & 0xf0) | 0x0c;

		tp += 8;
		for(char k=0; k<w; k++)
		{
			for(char j=0; j<8; j++)
				tp[j] = 0x00;
			tp += 8;
		}

		for(char j=0; j<8; j+=2)
		{
			tp[j + 0] = (tp[j + 0] & 0x0c) | 0x30;
			tp[j + 1] = (tp[j + 1] & 0x03) | 0x30;
		}

		wp += 320;
	}

	tp = wp;
	tp[0] = (tp[0] & 0xf0) | 0x0c;
	tp[1] = (tp[1] & 0xf0) | 0x0c;
	tp[2] |= 0x0f;
	tp[3] |= 0x0f;

	tp += 8;
	for(char i=0; i<w; i++)
	{
		tp[0] = 0x00;
		tp[1] = 0x00;
		tp[2] = 0xff;
		tp[3] = 0xff;
		tp[4] &= 0xcc;
		tp[5] &= 0x33;
		tp[6] &= 0xcc;
		tp[7] &= 0x33;
		tp += 8;
	}

	tp[0] = (tp[0] & 0x0c) | 0x30;
	tp[1] = (tp[1] & 0x03) | 0x30;
	tp[2] = (tp[2] & 0x0c) | 0xf0;
	tp[3] = (tp[3] & 0x03) | 0xf0;
	tp[4] &= 0xcc;
	tp[5] &= 0x33;
	tp[6] &= 0xcc;
	tp[7] &= 0x33;

	window_fill(0x00);
}

void window_close(void)
{
	grid_redraw_rect(winX - 1, winY - 1, winW + 2, winH + 2)

//	drawBaseGrid();
//	updateColors();
}

