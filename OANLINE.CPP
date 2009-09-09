// Filename    : OANLINE.CPP
// Description : animated line
// Onwership   : Gilbert

#include <OANLINE.H>
#include <OVGABUF.H>
#include <ALL.H>
#include <math.h>

// ----------- Define color table
unsigned char AnimLine::init_color_code[ANIMCOLOR_PERIOD][ANIMLINE_PERIOD] =
{
//	{ 0x90, 0x93, 0x98, 0x9c, 0x9f, 0x9c, 0x98, 0x93 },
//	{ 0x90, 0x93, 0x97, 0x9a, 0x9d, 0x9a, 0x97, 0x93 },
//	{ 0x90, 0x92, 0x95, 0x97, 0x9a, 0x97, 0x95, 0x92 },
//	{ 0x90, 0x92, 0x95, 0x97, 0x9a, 0x97, 0x95, 0x92 },
//	{ 0x90, 0x93, 0x97, 0x9a, 0x9d, 0x9a, 0x97, 0x93 },
//	{ 0x90, 0x93, 0x98, 0x9c, 0x9f, 0x9c, 0x98, 0x93 }

	{ 0xa6, 0xa5, 0xa4, 0xa4, 0x9c, 0xa4, 0xa4, 0xa5 },
	{ 0xa7, 0xa6, 0xa5, 0xa4, 0x9c, 0xa4, 0xa5, 0xa6 },
	{ 0xc7, 0xa7, 0xa6, 0x9c, 0x9f, 0x9c, 0xa6, 0xa7 },
	{ 0x90, 0x93, 0x98, 0x9c, 0x9f, 0x9c, 0x98, 0x93 },
	{ 0xc7, 0xa7, 0xa6, 0x9c, 0x9f, 0x9c, 0xa6, 0xa7 },
	{ 0xa7, 0xa6, 0xa5, 0xa4, 0x9c, 0xa4, 0xa5, 0xa6 }
};

unsigned char AnimLine::series_color_code[ANIMCOLOR_SERIES][ANIMLINE_PERIOD] =
{
	{ 0x90, 0x93, 0x98, 0x9c, 0x9f, 0x9c, 0x98, 0x93 },
	// { 0xa6, 0xa5, 0xa4, 0x9c, 0x9f, 0x9c, 0xa4, 0xa5 },
	{ 0xA0, 0xA1, 0xA2, 0xA3, 0xC3, 0xA3, 0xA2, 0xA0 },
	{ 0xA0, 0xC3, 0xA0, 0xC3, 0xA0, 0xC3, 0xA0, 0xC3 },
};

// ----------- Begin of function AnimLine::init ----------//
//
// <short> x1, y1, x2, y2    defines corners of the display area
//
void AnimLine::init(short x1, short y1, short x2, short y2)
{
	// set the boundary
	bound_x1 = x1;
	bound_y1 = y1;
	bound_x2 = x2;
	bound_y2 = y2;
	phase = 0;
	color_phase = 0;
}
// ----------- End of function AnimLine::init ----------//


// ----------- Begin of function AnimLine::inc_phase ----------//
//
// called after each display cycle
//
void AnimLine::inc_phase()
{
	phase = (phase == 0 ? ANIMLINE_PERIOD : phase) - 1;
	if( ++color_phase >= ANIMCOLOR_PERIOD*ANIMCOLOR_INNER_PERIOD )
		color_phase = 0;
}
// ----------- End of function AnimLine::inc_phase ----------//


// ----------- Begin of function AnimLine::draw_line ----------//
//
// clip the line before calling basic_line, basic_hline or basic_vline to draw
// animatedFlag = TRUE for animated line, FALSE for stable line
//
// <VgaBuf *>vgabuf      destination vgabuf
// <short> x1,y1,x2,y2   draw from (x1,y1) to (x2,y2) on the screen
// [int] animatedFlag    whether the colors of the line is cycle   (default:1 = animated)
// [int] effectFlag      effect : -1=dimming B/W, 0=non-dimming B/W, 1=non-dimming series (default : 0)
//
void AnimLine::draw_line(VgaBuf *vgabuf, short x1, short y1, short x2, short y2, int animatedFlag, int effectFlag)
{
	if( (x1 < bound_x1 && x2 < bound_x1) || ( x1 > bound_x2 && x2 > bound_x2) ||
		(y1 < bound_y1 && y2 < bound_y1) || ( y1 > bound_y2 && y2 > bound_y2) )
		return;

	unsigned char* colorCode;
	if( effectFlag == -1)
		colorCode = init_color_code[color_phase/ANIMCOLOR_INNER_PERIOD];
	else if( effectFlag >= 0 && effectFlag < ANIMCOLOR_SERIES)
		colorCode = series_color_code[effectFlag];
	else
	{
		err_here();		// invalid effectFlag
		colorCode = init_color_code[0];
	}

	if( x1 >= bound_x1 && x1 <= bound_x2 && x2 >= bound_x1 && x2 <= bound_x2 &&
		y1 >= bound_y1 && y1 <= bound_y2 && y2 >= bound_y1 && y2 <= bound_y2)
	{
		// already inside the screen
		if( y1 == y2)
		{
			basic_hline(vgabuf, x1, x2, y2, animatedFlag, colorCode);
		}
		else if( x1 == x2)
		{
			basic_vline(vgabuf, x1, y1, y2, animatedFlag, colorCode);
		}
		else
		{
			basic_line(vgabuf, x1, y1, x2, y2, animatedFlag, colorCode);
		}
	}
	else
	{
		if( y1 == y2)
		{
			// horizontal line
			if( y1 < bound_y1 || y1 > bound_y2)
				return;
			if( x1 < bound_x1)
				x1 = bound_x1;
			else if( x1 > bound_x2)
				x1 = bound_x2;

			if( x2 < bound_x1)
				x2 = bound_x1;
			else if (x2 > bound_x2)
				x2 = bound_x2;

			basic_hline(vgabuf, x1, x2, y1, animatedFlag, colorCode);
		}
		else if( x1 == x2)
		{
			// vertical line
			if( x1 < bound_x1 || x1 > bound_x2)
				return;
			if( y1 < bound_y1)
				y1 = bound_y1;
			else if( y1 > bound_y2)
				y1 = bound_y2;

			if( y2 < bound_y1)
				y2 = bound_y1;
			else if( y2 > bound_y2)
				y2 = bound_y2;

			basic_vline(vgabuf, x1, y1, y2, animatedFlag, colorCode);
		}
		else
		{
			// neither horizontal or vertical
			int	x1a, y1a, x1b, y1b;
			int	x2a, y2a, x2b, y2b;

			// find intersection points for the first point
			x1a = x1b = x1;
			y1a = y1b = y1;
			if( y1 < bound_y1)
			{
				y1a = bound_y1;
				x1a = top_intercept( x1, y1, x2, y2);
			}
			if( y1 > bound_y2)
			{
				y1a = bound_y2;
				x1a = bottom_intercept(x1, y1 ,x2, y2);
			}
			if( x1 < bound_x1)
			{
				x1b = bound_x1;
				y1b = left_intercept(x1, y1, x2, y2);
			}
			if( x1 > bound_x2)
			{
				x1b = bound_x2;
				y1b = right_intercept( x1, y1, x2, y2);
			}
			
			// find intersection points for the second point
			x2a = x2b = x2;
			y2a = y2b = y2;
			if( y2 < bound_y1)
			{
				y2a = bound_y1;
				x2a = top_intercept( x1, y1, x2, y2);
			}
			if( y2 > bound_y2)
			{
				y2a = bound_y2;
				x2a = bottom_intercept(x1, y1 ,x2, y2);
			}
			if( x2 < bound_x1)
			{
				x2b = bound_x1;
				y2b = left_intercept(x1, y1, x2, y2);
			}
			if( x2 > bound_x2)
			{
				x2b = bound_x2;
				y2b = right_intercept( x1, y1, x2, y2);
			}

			// replace x1, y1
			if( x1 < bound_x1 || x1 > bound_x2 || y1 < bound_y1 || y1 > bound_y2)
			{
				if( x1a < bound_x1 || x1a > bound_x2 || y1a < bound_y1 || y1a > bound_y2)
				{
					if( x1b < bound_x1 || x1b > bound_x2 || y1b < bound_y1 || y1b > bound_y2)
					{
						return;
					}
					else
					{
						x1 = x1b;
						y1 = y1b;
					}	
				}
				else
				{
					x1 = x1a;
					y1 = y1a;
				}
			}

			// replace x2, y2
			if( x2 < bound_x1 || x2 > bound_x2 || y2 < bound_y1 || y2 > bound_y2)
			{
				if( x2a < bound_x1 || x2a > bound_x2 || y2a < bound_y1 || y2a > bound_y2)
				{
					if( x2b < bound_x1 || x2b > bound_x2 || y2b < bound_y1 || y2b > bound_y2)
					{
						return;
					}
					else
					{
						x2 = x2b;
						y2 = y2b;
					}	
				}
				else
				{
					x2 = x2a;
					y2 = y2a;
				}
			}

			// a segment may be horizontal or vertical, check again
//			if( y1 == y2)
//				basic_hline(vgabuf, x1, x2, y1, animatedFlag);
//			else if( x1 == x2)
//				basic_vline(vgabuf, x1, y1, y2, animatedFlag);
//			else
			basic_line(vgabuf, x1, y1, x2, y2, animatedFlag, colorCode);
		}
	}
}
// ----------- End of function AnimLine::draw_line ----------//

// ----------- Begin of function AnimLine::thick_line ----------//
//
// draw a thicker line
//
// <VgaBuf *>vgabuf      destination vgabuf
// <short> x1,y1,x2,y2   draw from (x1,y1) to (x2,y2) on the screen
// [int] animatedFlag    whether the colors of the line is cycle   (default:1 = animated)
// [int] effectFlag      effect : -1=dimming B/W, 0=non-dimming B/W, 1=non-dimming series (default : 0)
//
void AnimLine::thick_line(VgaBuf *vgabuf, short x1, short y1, short x2, short y2, int animatedFlag, int effectFlag)
{
	if( abs(x2-x1) > abs(y2-y1) )
	{
		// likely to be horizontal, draw top, bottom and centre
		draw_line( vgabuf, x1, y1-1, x2, y2-1, animatedFlag, effectFlag);
		draw_line( vgabuf, x1, y1+1, x2, y2+1, animatedFlag, effectFlag);
	}
	else
	{
		// likely to be vertical, draw left right and centre
		draw_line( vgabuf, x1-1, y1, x2-1, y2, animatedFlag, effectFlag);
		draw_line( vgabuf, x1+1, y1, x2+1, y2, animatedFlag, effectFlag);
	}
	draw_line( vgabuf, x1, y1, x2, y2, animatedFlag, effectFlag);
}
// ----------- End of function AnimLine::thick_line ----------//


// ----------- Begin of function AnimLine::basic_line ----------//
//
// draw a clipped line
//
void AnimLine::basic_line(VgaBuf *vgabuf, short x1, short y1, short x2, short y2, int animatedFlag,
	unsigned char *colorCode)
{
	err_when( x1 < bound_x1 || x1 > bound_x2 );
	err_when( x2 < bound_x1 || x2 > bound_x2 );
	err_when( y1 < bound_y1 || y1 > bound_y2 );
	err_when( y2 < bound_y1 || y2 > bound_y2 );

	int	dx = x2 - x1;
	int	dy = y2 - y1;

	if( dy == 0)
	{
		basic_hline(vgabuf, x1, x2, y1, animatedFlag, colorCode);
		return;
	}

	if( dx == 0)
	{
		basic_vline(vgabuf, x1, y1, y2, animatedFlag, colorCode);
		return;
	}
	int	d;
	int	inc_x = dx > 0 ? 1 : -1;
	int	inc_y = dy > 0 ? 1 : -1;
	int	lPitch = dy > 0 ? vgabuf->buf_pitch() : -vgabuf->buf_pitch();
	unsigned char *bufPtr = (unsigned char *) vgabuf->buf_ptr(x1,y1);
	short	linePhase;
	err_when( dx == 0 || dy == 0);

	if( abs(dy) <= abs(dx) )
	{
		// draw gentle line
		// use x as independent variable
		dx = abs(dx);
		dy = abs(dy);

		d = 2 * dy - dx;	
		int x = x1-inc_x;
		linePhase = animatedFlag ? phase : 0;
		do
		{
			x += inc_x;
			*bufPtr = colorCode[linePhase];
			if(++linePhase >= ANIMLINE_PERIOD )
				linePhase = 0;
			bufPtr += inc_x;
			if( d >= 0)
			{
				// y increase by 1
				bufPtr += lPitch;
				d += 2 * (dy - dx);
			}
			else
			{
				// y remain unchange;
				d += 2 * dy;
			}
		} while ( x != x2);
	}
	else
	{
		// draw steep line
		// use y as independent variable
		dx = abs(dx);
		dy = abs(dy);

		d = 2 * dx - dy;
		int y = y1 - inc_y;
		linePhase = animatedFlag ? phase : 0;
		do
		{
			y += inc_y;
			*bufPtr = colorCode[linePhase];
			if(++linePhase >= ANIMLINE_PERIOD )
				linePhase = 0;
			bufPtr += lPitch;
			if( d >= 0)
			{
				// x increase by 1
				bufPtr += inc_x;
				d += 2 * (dx - dy);
			}
			else
			{
				// x remain unchange;
				d += 2 * dx;
			}
		} while ( y != y2);
	}
}
// ----------- End of function AnimLine::basic_line ----------//

// ----------- Begin of function AnimLine::basic_hline ----------//
//
// draw a clipped horizontal line
//
void AnimLine::basic_hline(VgaBuf *vgabuf, short x1, short x2, short y1, int animatedFlag,
	unsigned char *colorCode)
{
	err_when( x1 < bound_x1 || x1 > bound_x2 );
	err_when( x2 < bound_x1 || x2 > bound_x2 );
	err_when( y1 < bound_y1 || y1 > bound_y2 );

	short linePhase = animatedFlag ? phase : 0;
	unsigned char *bufPtr = (unsigned char *)vgabuf->buf_ptr(x1, y1);

	if( x1 <= x2)
	{
		// from left to right
		for(short x = x1; x <= x2; ++x, ++bufPtr)
		{
			*bufPtr = colorCode[linePhase];
			if(++linePhase >= ANIMLINE_PERIOD )
				linePhase = 0;
		}
	}
	else
	{
		// from right to left
		for( short x = x1; x >= x2; --x, --bufPtr)
		{
			*bufPtr = colorCode[linePhase];
			if(++linePhase >= ANIMLINE_PERIOD )
				linePhase = 0;
		}
	}
}
// ----------- End of function AnimLine::basic_hline ----------//

// ----------- Begin of function AnimLine::basic_vline ----------//
//
// draw a clipped vertical line
//
void AnimLine::basic_vline(VgaBuf *vgabuf, short x1, short y1, short y2, int animatedFlag,
	unsigned char *colorCode)
{
	err_when( x1 < bound_x1 || x1 > bound_x2 );
	err_when( y1 < bound_y1 || y1 > bound_y2 );
	err_when( y2 < bound_y1 || y2 > bound_y2 );

	short linePhase = animatedFlag ? phase : 0;
	unsigned char *bufPtr = (unsigned char *) vgabuf->buf_ptr(x1, y1);
	int	lPitch = vgabuf->buf_pitch();

	if( y1 <= y2)
	{
		// from top to bottom
		for(short y = y1; y <= y2; ++y, bufPtr += lPitch)
		{
			*bufPtr = colorCode[linePhase];
			if(++linePhase >= ANIMLINE_PERIOD )
				linePhase = 0;
		}
	}
	else
	{
		// from bottom to top
		for( short y = y1; y >= y2; --y, bufPtr -= lPitch)
		{
			*bufPtr = colorCode[linePhase];
			if(++linePhase >= ANIMLINE_PERIOD )
				linePhase = 0;
		}
	}
}
// ----------- End of function AnimLine::basic_vline ----------//


// ----------- Begin of function AnimLine::get_series_color_array ----------//
unsigned char *AnimLine::get_series_color_array(int effectFlag)
{
	unsigned char *colorCode;
	if( effectFlag == -1)
		colorCode = init_color_code[color_phase/ANIMCOLOR_INNER_PERIOD];
	else if( effectFlag >= 0 && effectFlag < ANIMCOLOR_SERIES)
		colorCode = series_color_code[effectFlag];
	else
	{
		err_here();		// invalid effectFlag
		colorCode = init_color_code[0];
	}

	return colorCode;
}
// ----------- End of function AnimLine::get_series_color_array ----------//

// ----------- Begin of function AnimLine::top_intercept ----------//
short AnimLine::top_intercept( short x1, short y1, short x2, short y2)
{
	return (bound_y1-y1) * (x2-x1) / (y2-y1) + x1;
}
// ----------- End of function AnimLine::top_intercept ----------//


// ----------- Begin of function AnimLine::bottom_intercept ----------//
short AnimLine::bottom_intercept( short x1, short y1, short x2, short y2)
{
	return (bound_y2-y1) * (x2-x1) / (y2-y1) + x1;
}
// ----------- End of function AnimLine::bottom_intercept ----------//


// ----------- Begin of function AnimLine::left_intercept ----------//
short AnimLine::left_intercept( short x1, short y1, short x2, short y2)
{
	return (bound_x1-x1) * (y2-y1) / (x2-x1) + y1;
}
// ----------- End of function AnimLine::left_intercept ----------//


// ----------- Begin of function AnimLine::right_intercept ----------//
short AnimLine::right_intercept( short x1, short y1, short x2, short y2)
{
	return (bound_x2-x1) * (y2-y1) / (x2-x1) + y1;
}
// ----------- End of function AnimLine::right_intercept ----------//
