
#include "perlin.h"

inline unsigned hashIntWard(unsigned k) 
{
	unsigned ks = k;
	ks ^= ks << 7;
    ks ^= ks >> 9;
    ks ^= ks << 8;
	return ks;
}

static byte shuffle[256];

// using 8 fractional bits

int grad2(byte u, byte v, int x, int y) 
{
	unsigned h = hashIntWard(shuffle[u]  | (shuffle[v] << 8));
	return x * ((int)(h & 127) - 64) + y * ((int)((h >> 8) & 127) - 64);
}

byte ir[32] = {
	0, 0, 1, 2, 4, 8, 12, 19,
	27, 36, 46, 58, 70, 84, 98, 113,
	128, 143, 158, 172, 186, 198, 210, 220,
	230, 237, 244, 248, 252, 254, 255, 255
};

// using 8 fractional bits

inline int interpolate2(byte x, byte y, int w00, int w10, int w01, int w11) 
{
	int sx = ir[(char)(x >> 3)];
	int sy = ir[(char)(y >> 3)];

	int w0 = w00 + sx * ((w10 - w00) >> 8);
	int w1 = w01 + sx * ((w11 - w01) >> 8);
	return w0 + sy * ((w1 - w0) >> 8);
}

inline int noise2(int x, int y) 
{
	byte ix = x >> 8, iy = y >> 8;
	int rx = x & 0xff;
	int ry = y & 0xff;

	int w00 = grad2(ix    , iy    , rx      , ry      );
	int w10 = grad2(ix + 1, iy    , rx - 256, ry      );
	int w01 = grad2(ix    , iy + 1, rx      , ry - 256);
	int w11 = grad2(ix + 1, iy + 1, rx - 256, ry - 256);
	return interpolate2(rx, ry, w00, w10, w01, w11);
}

void mnoise_init(unsigned seed)
{
	unsigned s = seed;
	for(unsigned b=0; b<256; b++)
		shuffle[b] = b;
	for(unsigned b=0; b<256; b++)
	{
		s ^= s << 7;
	    s ^= s >> 9;
    	s ^= s << 8;

		byte a = s & 0xff;
		byte x = shuffle[a]; shuffle[a] = shuffle[(byte)b]; shuffle[(byte)b] = x;
	}
}

int mnoise2(int x, int y, byte noct, int phi) 
{
	int	sum = 0;
	int	a = 128;

	do {
		sum += a * (noise2(x, y) >> 6);
		x += x;
		y += y;
		noct--;
		a = (a * phi) >> 8;
	} while (noct > 0);

	return sum;
}
