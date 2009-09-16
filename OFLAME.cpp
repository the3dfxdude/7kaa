// Filename   : OFLAME.CPP
// Description: class Flame
// Onwership  : Gilbert

// Frank Jan Sorensen Alias:Frank Patxi (fjs@lab.jt.dk) for the
// fireroutine.

#include <OFLAME.H>
#include <OVGABUF.H>
#include <ALL.H>
#include <COLCODE.H>


//-------------- Begin Function Flame::Flame ----------//
Flame::Flame()
{
	heat_map = 0;
	bitmap = 0;
}
//-------------- End Function Flame::Flame ----------//


//-------------- Begin Function Flame::Flame ----------//
Flame::Flame(short width, short height, short flameWidth, FlameType flameType)
{
	heat_map = 0;
	bitmap = 0;
	init( width, height, flameWidth, flameType);
}
//-------------- End Function Flame::Flame ----------//


//-------------- Begin Function Flame::~Flame ----------//
Flame::~Flame()
{
	deinit();
}
//-------------- End Function Flame::~Flame ----------//


//-------------- Begin Function Flame::init ----------//
//
// <short> width             width of the bitmap
// <short> height            height of the bitmap
// <short> flameWdith        length of flame source at base line
// <FlameType> flameType     determine how flame source is generated at base line
//
void Flame::init(short width, short height, short flameWidth, FlameType flameType)
{
	map_width = width;
	map_height = height;

	seed = (width * height + flameWidth) | 1;
	// allocated more space for overflowing during rise()
	heat_map = (unsigned char *) mem_resize(heat_map, width * height + 4);

	memset(heat_map, 0, width*height);

	bitmap = (unsigned char *) mem_resize(bitmap, width*height + 2*sizeof(short) );

	memset(bitmap+2*sizeof(short), width*height, 255);
	*(short *)bitmap = width;
	*(((short *)bitmap)+1) = height;

	smooth = 16;
	shade_base = -1;
	decay = 800/ height;
	if( decay < 2)
		decay = 2;

	// ---------- put flame sources
	switch(flameType)
	{
	case FLAME_CENTRE_POINT:
		{
			short i = width/2;
			heat_map[i] = 250;

			//-------- smoothing around
			for( short j = 1 ; j < smooth; ++j)
			{
				unsigned char k = 250 * (smooth-j)/smooth;
				if( i-j >= 0)
					heat_map[i-j] = k;
				if( i+j < width)
					heat_map[i+j] = k;
			}
		}
		break;

	case FLAME_RANDOM_POINTS:
		{
			for(short n = flameWidth*4/smooth; n > 0; --n)
			{
				short i = (width-flameWidth)/2+random(flameWidth) ;
				heat_map[i] = 250;

				//-------- smoothing around
				for( short j = 1 ; j < smooth; ++j)
				{
					unsigned char k = 250 * (smooth-j)/smooth;
					if( i-j >= 0 && k>heat_map[i-j])
						heat_map[i-j] = k;
					if( i+j < width && k>heat_map[i+j])
						heat_map[i+j] = k;
				}
			}
		}
		break;

	case FLAME_WIDE:
		{
			short left = (width - flameWidth)/2;
			short right = left + flameWidth;
			memset(heat_map+left, 250, flameWidth);
			for( short j = 1; j < smooth; ++j)
			{
				unsigned char k = 250 * (smooth-j)/smooth;
				if( left-j >= 0)
					heat_map[left-j] = k;
				if( right+j <= width)
					heat_map[right+j-1] = k;
			}

		}
		break;
	default:
		err_now("undefined FlameType");
	}
}
//-------------- End Function Flame::init ----------//


//-------------- Begin Function Flame::deinit ----------//
void Flame::deinit()
{
	if(heat_map)
	{
		mem_del(heat_map);
		heat_map=NULL;
	}

	if(bitmap)
	{
		mem_del(bitmap);
		bitmap=NULL;
	}
}
//-------------- End Function Flame::deinit ----------//


//-------------- Begin Function Flame::operator= ----------//
Flame& Flame::operator= (Flame &f)
{
	if(heat_map && f.heat_map && map_width*map_height == f.map_width*f.map_height)
	{
		memcpy(heat_map, f.heat_map, f.map_width*f.map_height);
	}
	else
	{
		if(heat_map)
		{
			mem_del(heat_map);
			heat_map = NULL;
		}
		if(f.heat_map)
		{
			heat_map = (unsigned char *) mem_add(f.map_width*f.map_height+4);
			memcpy(heat_map, f.heat_map, f.map_width*f.map_height);
		}
	}

	if(bitmap && f.bitmap && map_width*map_height == f.map_width*f.map_height)
	{
		memcpy(bitmap, f.bitmap, f.map_width*f.map_height + 2*sizeof(short));
	}
	else
	{
		if(bitmap)
		{
			mem_del(bitmap);
			bitmap = NULL;
		}
		if(f.bitmap)
		{
			bitmap = (unsigned char *) mem_add(f.map_width*f.map_height+2*sizeof(short));
			memcpy(bitmap, f.bitmap, f.map_width*f.map_height + 2*sizeof(short));
		}
	}

	seed = f.seed;
	map_width = f.map_width;
	map_height = f.map_height;
	decay = f.decay;
	smooth = f.smooth;
	shade_base = f.shade_base;

	return(*this);
}
//-------------- End Function Flame::operator= ----------//


//-------------- Begin Function Flame::heat_up ----------//
void Flame::heat_up(short h)
{
	decay = h;
}
//-------------- End Function Flame::heat_up ----------//


//-------------- Begin Function Flame::rise ----------//
//
// change heat_map to next pattern
// <short> wind         -1 = blow left (west), 0 = center, +1 = blow right
//
void Flame::rise(short wind)
{
	// rise of each particle of frame
	unsigned char *p;			// pointer to heat_map
/*
	short x,y;
	
	// 1st version, left to right, top to bottom
	for( x=0; x < map_width; ++x)
	{
		y = map_height-2;
		p = heat_map + y*map_width + x;
		for( ; y >= 0; --y, p-=map_width )
		{
			unsigned char v = *p;
			if( v < decay || x <= 0 || x >= map_width-1)
			{
				p[map_width] = 0;					// the pixel just above
			}
			else
			{
				// draw a point on the buffer
				p[map_width -random(3)+1  + wind*random(2)] =
					v - random(decay);
			}
		}
	}
	
	 
	// 2nd version top to bottom, left to center and right to center
	for( y = map_height-2; y >= 0; --y)
	{
		// left to center
		p = heat_map + y * map_width;
		for( x = 0; x <= map_width/2; ++x, ++p)
		{
			unsigned char v = *p;
			if( v < decay || x <= 0)
			{
				p[map_width] = 0;				// the pixel just above
			}
			else
			{
				// draw a point on the buffer
				p[map_width -random(3)+1 + wind*random(2)] =
					v - random(decay);
			}
		}

		// right to center
		p = heat_map + y * map_width + map_width-1;
		for( x = map_width; x > map_width/2; --x, --p)
		{
			unsigned char v = *p;
			if( v < decay || x >= map_width-1)
			{
				p[map_width] = 0;				// the pixel just above
			}
			else
			{
				// draw a point on the buffer
				p[map_width -random(3)+1 + wind*random(2)] =
					v - random(decay);
			}
		}

	}
*/


	// 2nd version assembly version
	const BALANCE_PERIOD = 11;
	const WIND_PERIOD = 13;
	const DECAY_PERIOD = 17;
	char	balanceRnd[BALANCE_PERIOD];
	char	windRnd[WIND_PERIOD];
	char	decayRnd[DECAY_PERIOD];
	for(unsigned balanceN = 0; balanceN < BALANCE_PERIOD; ++balanceN)
	{
		int r = random(4);
		if(r >= 3)
			r = 2;
		balanceRnd[balanceN] = r-1;
	}
	for(unsigned windN = 0; windN < WIND_PERIOD; ++windN)
		windRnd[windN] = wind*random(2);
	for(unsigned decayN = 0; decayN < DECAY_PERIOD; ++decayN)
		decayRnd[decayN] = random(decay);
	balanceN = 0;
	windN = 0;
	decayN = 0;
	int	mapWidth = map_width;
	int	mapHeight = map_height;
	int	leftLen = map_width/2;
	int	rightLen = map_width - leftLen;
	p = heat_map + (map_height-2)*map_width;
	unsigned char Decay = (unsigned char) decay;
	_asm
	{
		mov	ecx, mapHeight
		dec	ecx
		mov	esi, p
	rise_1:
		push	ecx

		mov	ecx, leftLen
		; exclude left most dot
		dec	ecx
		mov	ebx, mapWidth
		mov	byte ptr [esi+ebx],0
		inc	esi
		cld
	rise_2:
		mov	edi, esi
		lodsb
		cmp	al, Decay
		jnb	rise_2a
		mov	ebx, mapWidth
		mov	[edi+ebx], 0
		jmp	rise_2b
	rise_2a:
		xor	ebx, ebx
		mov	edx, windN
		mov	bl, windRnd[edx]
		mov	edx, balanceN
		add	bl, balanceRnd[edx]
		mov	edx, decayN
		sub	al, decayRnd[edx]
		movsx	ebx, bl
		add	ebx, mapWidth
		mov	[edi+ebx], al

		; balanceN = (balanceN + 1) % BALANCE_PERIOD
		mov	edx, balanceN
		inc	edx
		cmp	edx, BALANCE_PERIOD
		jb		rise_2c
		xor	edx,edx
	rise_2c:
		mov	balanceN, edx

		mov	edx, windN
		inc	edx
		cmp	edx, WIND_PERIOD
		jb		rise_2d
		xor	edx,edx
	rise_2d:
		mov	windN, edx

		mov	edx, decayN
		inc	edx
		cmp	edx, DECAY_PERIOD
		jb		rise_2e
		xor	edx, edx
	rise_2e:
		mov	decayN, edx

	rise_2b:
		loop	rise_2

		mov	ecx, rightLen
		; ignore rightmost dot
		dec	ecx
		add	esi, rightLen
		dec	esi
		mov	ebx, mapWidth
		mov	byte ptr [esi+ebx],0
		dec	esi
		std
	rise_3:
		mov	edi, esi
		lodsb
		cmp	al, Decay
		jnb	rise_3a
		mov	ebx, mapWidth
		mov	[edi+ebx], 0
		jmp	rise_3b
	rise_3a:
		xor	ebx, ebx
		mov	edx, windN
		mov	bl, windRnd[edx]
		mov	edx, balanceN
		sub	bl, balanceRnd[edx]
		mov	edx, decayN
		sub	al, decayRnd[edx]
		movsx	ebx, bl
		add	ebx, mapWidth
		mov	[edi+ebx], al

		; balanceN = (balanceN + 1) % BALANCE_PERIOD
		mov	edx, balanceN
		inc	edx
		cmp	edx, BALANCE_PERIOD
		jb		rise_3c
		xor	edx,edx
	rise_3c:
		mov	balanceN, edx

		mov	edx, windN
		inc	edx
		cmp	edx, WIND_PERIOD
		jb		rise_3d
		xor	edx, edx
	rise_3d:
		mov	windN, edx

		mov	edx, decayN
		inc	edx
		cmp	edx, DECAY_PERIOD
		jb		rise_3e
		xor	edx, edx
	rise_3e:
		mov	decayN, edx

	rise_3b:
		loop	rise_3

		pop	ecx
		sub	esi, leftLen
		inc	esi
		sub	esi, mapWidth
		dec	ecx
		jz		rise_4
		jmp	rise_1
	rise_4:
		cld
	}
}
//-------------- End Function Flame::rise ----------//


//-------------- Begin Function Flame::gen_bitmap ----------//
//
// convert heat_map to transparent bitmap
// to be blt to vgabuf by put_bitmap_trans or put_bitmap_area_trans
//
// [unsigned char] shadeColor       first (and brightest) color of the color scale
//                                   (default: 0x10 for yellow-to-red flame)
//
void Flame::gen_bitmap(unsigned char shadeColor)
{
	// generate color code
	static unsigned char lastShadeBase = 0;
	static unsigned char colorTable[256];
/*	static unsigned char colorTable[256] = 
	{
		TRANSPARENT_CODE, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2,
		0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2,
		0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2,
		0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2, 0xd2,
		0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 
		0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 
		0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 
		0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 
		0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 
		0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 
		0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 
		0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 
		0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 
		0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
		0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
		0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
		0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
		0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
		0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
		0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 
		0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 
		0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 
		0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 
		0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
		0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 
		0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 
		0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 
		0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 0xeb, 
	};
	*/
	shade_base = shadeColor;
	if( shade_base != lastShadeBase )
	{
		lastShadeBase = (unsigned char)shade_base;
		unsigned char *ct = colorTable;
		*ct = TRANSPARENT_CODE;
		ct++;
		for(int b = 1; b <= 255; ++b, ct++)
		{
			*ct = shadeColor + (b >> 5);
		}
	}

	unsigned char *b = bitmap+2*sizeof(short);
	unsigned char *p = heat_map+(map_height-1)*map_width;
	short mapHeight = map_height;
	short mapWidth = map_width;
	_asm
	{
		mov	edi, b
		mov	esi, p
		cld
		movzx	ecx, mapHeight
		movzx	edx, mapWidth
		lea	ebx, colorTable
	gen_bitmap_loop2:
		push	ecx
		mov	ecx, edx
	gen_bitmap_loop3:
		lodsb
		xlatb	[ebx]
		stosb
		loop	gen_bitmap_loop3

		sub	esi, edx
		sub	esi, edx
		pop	ecx
		loop	gen_bitmap_loop2
	}
}
//-------------- End Function Flame::gen_bitmap ----------//


//-------------- Begin Function Flame::mask_bottom ----------//
//
// call after gen_bitmap,
// to round the bottom of bitmap
//
void Flame::mask_bottom()
{
	for( short x = 0; x < map_width; ++x)
	{
		short dx = x - map_width/2;
		short y;
		// fit a parabola
		// h = map_height/4 * (dx / (map_width/2))^2
		int h = map_height * dx * dx / (map_width * map_width);

		// modify it randomly
		if( h > 0)
			h += random(3)-1;		// -1 to +1
		else
			h += random(16)/8;			// 0 or +1;

		unsigned char *p = bitmap+2*sizeof(short)+(map_height-1)*map_width+x;
		for(y = 0; y < h; ++y)
		{
			*p = TRANSPARENT_CODE;
			p -= map_width;
		}
	}
}
//-------------- End Function Flame::mask_bottom ----------//


//-------------- Begin Function Flame::mask_transparent ----------//
//
// call after gen_bitmap,
// mask bitmap out every one out of two points to tranparent
//
void Flame::mask_transparent()
{
	unsigned char *b = bitmap + 2*sizeof(short);
	if( map_width & 1)
	{
		// odd number
		/*
		for( short y = 0; y < map_height; ++y)
			for( short x = y & 1; x < map_width; x += 2)
		{
			*b = 255;
			b += 2;
		}
		*/
		int len = map_height * map_width /2;
		_asm
		{
			mov	al, TRANSPARENT_CODE
			mov	edi, b
			mov	ecx, len
			cld
		mask_trans_loop1:
			stosb
			inc	edi
			loop	mask_trans_loop1
		}
	}
	else
	{
		// even number
		/*
		for( short y = 0; y < map_height; ++y)
		{
			b+= y & 1;		// shift one byte on odd line
			for( short x = y & 1; x < map_width; x += 2)
			{
				*b = 255;
				b += 2;
			}
			b-= y & 1;		// shift back to even dot.
		}
		*/
		int	mapHeight = map_height, mapWidth = map_width/2;
		_asm
		{
			mov	ecx, mapHeight
			mov	edi, b
			mov	al, TRANSPARENT_CODE
			mov	edx,0
		mask_trans_loop2:
			push	ecx
			add	edi, edx
			mov	ecx, mapWidth
		mask_trans_loop3:
			stosb
			inc	edi
			loop	mask_trans_loop3
			sub	edi, edx
			pop	ecx
			xor	edx,1
			loop	mask_trans_loop2
		}
	}
}
//-------------- End Function Flame::mask_transparent ----------//


//-------------- Begin Function Flame::draw_step ----------//
void Flame::draw_step(short left, short bottom, VgaBuf *vgabuf, short wind)
{
	// rise of each particle of frame
	unsigned char *p;			// pointer to heat_map
	unsigned char *d;			// pointer to vgabuf
	short x,y;
	for( x=0; x < map_width; ++x)
	{
		y = map_height-2;
		p = heat_map + y*map_width + x;
		d = (unsigned char *)vgabuf->buf_ptr() + 
			(bottom-y)*vgabuf->buf_pitch() + left + x;
		for( ; y >= 0; --y, p-=map_width, d+=vgabuf->buf_pitch() )
		{
			unsigned char v = *p;
			if( /* v == 0 || */ v < decay || x <= 0 || x >= map_width-1)
			{
				p[map_width] = 0;					// the pixel just above
			}
			else
			{
				unsigned char n = v - random(decay);
				p[map_width -random(3)+1  + wind*random(2)] = n;
				// draw a point on the buffer
				*d = shade_base + 0x0f - ( n >> 4);
			}
		}
	}
}
//-------------- End Function Flame::mask_transparent ----------//


//-------------- Begin Function Flame::flush_point ---------//
void Flame::flush_point(short x, short y)
{
	if( x < 0)
		x = random(map_width/2) + random(map_width/2);
	if( y < 1)
		y = 1 + random(map_height/4);

	unsigned char *center = heat_map + y*map_width + x;
	*center >>= 3;
	center[-1] >>= 2;			// left
	if( x-2 >= 0)
		center[-2] >>= 1;
	center[1] >>= 2;			// right
	if( x+2 < map_width)
		center[2] >>= 1;
	center[map_width] >>= 2;	// top
	center[map_width-1] >>= 1;
	center[map_width+1] >>= 1;
}

//-------------- End Function Flame::flush_point ---------//


//-------------- Begin Function Flame::random ---------//
unsigned Flame::random(unsigned bound)
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed % bound;

}

//-------------- End Function Flame::random ---------//
