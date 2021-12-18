#include "window.h"

const byte colorfont[] = {

#embed "colorfont.bin"

};

char	winX, winY, winW, winH;

void window_fill(char pat)
{
	char	*	wp = Hires + 320 * winY + 8 * winX;
	char		w = 8 * winW;
	
	for(char y=0; y<winH; y++)
	{
		for(char x=0; x<w; x++)
			wp[x] = pat;

		wp += 320;
	}
}

void window_scroll(void)
{
	char	*	wp = Hires + 320 * winY + 8 * winX;
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
	char	*	wp = Hires + 320 * (winY + y) + 8 * (winX + x);


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

void window_put_sprite(char x, char y, const char * sprite)
{
	char	*	wp = Hires + 320 * (winY + (y >> 3)) + 8 * (winX + x) + (y & 7);

	char	ry = y & 7;

	for(char iy=0; iy<21; iy++)
	{
		wp[0] = sprite[0]; wp[8] = sprite[1]; wp[16] = sprite[2];
		sprite += 3;
		wp++;
		ry++;
		if (ry == 8)
		{
			wp += 312;
			ry = 0;
		}
	}
}

void window_open(char x, char y, char w, char h)
{
	winX = x;
	winY = y;
	winW = w;
	winH = h;

	char	*	wp = Hires + 320 * (winY - 1) + 8 * (winX - 1);

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
	drawBaseGrid();
	updateColors();
	updateBaseGrid();
}

