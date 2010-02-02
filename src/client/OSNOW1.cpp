/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Filename   : OSNOW1.CPP
// Description: class SnowLayer
// Onwership  : Gilbert

#include <OSNOW.h>
#include <OVGABUF.h>
#include <COLOR.h>
#include <ALL.h>

//---------- Define constant -------------//

// #### begin Gilbert 8/5 ######//
const char SNOW_COLOUR1 = char(VGA_GRAY+8);
const char SNOW_COLOUR2 = char(VGA_GRAY+11);
const char SNOW_COLOUR3 = char(VGA_GRAY+14);
// #### end Gilbert 8/5 ######//

//---------- Begin of function SnowLayer::set_bound -----//
//
void SnowLayer::set_bound(int x1, int y1, int x2, int y2)
{
	bound_x1 = x1;
	bound_y1 = y1;
	bound_x2 = x2;
	bound_y2 = y2;
}
//---------- End of function SnowLayer::set_bound -----//

//---------- Begin of function SnowLayer::init -----//
//
void SnowLayer::init(short h, short v, short speed, short amp, short r,
							double s, char animSpeed, short initPeriod)
{
	h_sep = h;
	v_sep = v;
	seed = r + h*2 + v*5 + speed*7;
	fall_speed = speed;
	amplitude = amp;
	radius = r;
	snow_x = bound_x1 + random(h_sep) + r;
	snow_y = bound_y1 + r -1;
	slide_speed = (short)(s * fall_speed);
	blind_site = (char)(h + v) % 3;		// random number?
	anim_speed = animSpeed;
	anim_phase = 0;
	period = initPeriod;
}
//---------- End of function SnowLayer::init -----//

//---------- Begin of function SnowLayer::fall -----//
//
void SnowLayer::fall()
{
	for( anim_phase += anim_speed; anim_phase >= 10; anim_phase -= 10)
	{
		snow_y = snow_y + fall_speed;
		while( snow_y - radius - bound_y1 >= v_sep)
		{
			snow_y -= v_sep;
			blind_site = (blind_site + 1) % 3;
			if(++period > 1000)
				period = 1000;
		}
		snow_x += random(amplitude * 2 + 1) - amplitude + slide_speed;

		if( snow_x - radius < bound_x1 )
		{
			snow_x += h_sep;
			blind_site = (blind_site + 2) % 3;
		}
		else if( snow_x - radius - bound_x1 >= h_sep)
		{
			snow_x -= h_sep;
			blind_site = (blind_site + 1) % 3;
		}
	}
}
//---------- End of function SnowLayer::fall -----//

//---------- Begin of function SnowLayer::draw_step -----//
// note : blind_site is ranging from 0 to 2.
//  if the total (cx+cy) % 3 is equal to blind_site, the snow
//  is not displayed.
//  Therefore one third of the snow is not displayed.
//  The snow layer doesn't look too regular.
//
//  blind_site is adjusted in SnowLayer::fall() such that
//  snow doesn't disappear suddenly.
//
void SnowLayer::draw_step(VgaBuf *vgabuf)
{
	int sx, sy, cx, cy;
	int vPitch = vgabuf->buf_pitch();
	char *dotPt;
	fall();
	switch(radius)
	{
	case 1:
		for(sy = snow_y, cy=0; sy < bound_y2 && cy < period; sy+= v_sep, ++cy)
		{
			for( sx = snow_x, cx=0; sx < bound_x2; sx += h_sep, ++cx)
			{
				if((cx+cy) % 3 != blind_site)
				{
					dotPt = vgabuf->buf_ptr() + vPitch*(sy-1) + sx-1;
					*dotPt++ = SNOW_COLOUR1;
					*dotPt++ = SNOW_COLOUR2;
					*dotPt   = SNOW_COLOUR1;

					dotPt += vPitch - 2;
					*dotPt++ = SNOW_COLOUR2;
					*dotPt++ = SNOW_COLOUR3;
					*dotPt   = SNOW_COLOUR2;

					dotPt += vPitch - 2;
					*dotPt++ = SNOW_COLOUR1;
					*dotPt++ = SNOW_COLOUR2;
					*dotPt   = SNOW_COLOUR1;
				}
			}
		}
	case 0:
		for(sy = snow_y,cy =0; sy <= bound_y2 && cy < period; sy+= v_sep, ++cy)
		{
			for( sx = snow_x, cx=0; sx <= bound_x2; sx += h_sep, ++cx)
			{
				if( (cx+cy) % 3 != blind_site)
				{
					vgabuf->draw_pixel(sx, sy, SNOW_COLOUR3);
				}
			}
		}
		break;
	default:
		err_now("undefined snow radius");
	}
}
//---------- End of function SnowLayer::draw_step -----//

//---------- Begin of function SnowLayer::random -----//
unsigned SnowLayer::random(unsigned bound)
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed % bound;
}
//---------- End of function SnowLayer::random -----//

