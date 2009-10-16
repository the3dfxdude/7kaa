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

// Filename    : OLIGHTN.CPP
// Description : Lightning class
// Ownership   : Gilbert

#include	<math.h>
#include <time.h>
#include <OVGABUF.h>
#include <OMISC.h>
#include <OLIGHTN.h>
#include <OWORLDMT.h>
#include <COLOR.h>

//------------ Define constant ----------//

#define PI 3.141592654

#define CORELIGHTNCOLOR (VGA_GRAY+15)
#define INLIGHTNCOLOR (VGA_GRAY+13)
#define OUTLIGHTNCOLOR (VGA_GRAY+10)

//--------- Define static class vars --------//

int Lightning::bound_x1 = ZOOM_X1+4;
int Lightning::bound_y1 = ZOOM_Y1-4;
int Lightning::bound_x2 = ZOOM_X2-4;
int Lightning::bound_y2 = ZOOM_Y2-4;

//---------- Declare static functions ----------//

static double sqr(double x);

//-------- Begin of static function Lightning::dist ----------//
double Lightning::dist(double dx, double dy)
{
	return sqrt( dx*dx + dy*dy);
}
//-------- End of static function Lightning::dist ----------//

//-------- Begin of static function Lightning::set_clip ----------//
void Lightning::set_clip(int x1, int y1, int x2, int y2)
{
	bound_x1 = x1;
	bound_x2 = x2;
	bound_y1 = y1;
	bound_y2 = y2;
}
//-------- End of static function Lightning::set_clip ----------//

//-------- Begin of function Lightning::~Lightning ----------//
Lightning::~Lightning()
{
}
//-------- End of function Lightning::~Lightning ----------//

//-------- Begin of function Lightning::rand_seed ----------//
unsigned Lightning::rand_seed()
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed;
}
//-------- End of function Lightning::rand_seed ----------//

//-------- Begin of function Lightning::init ----------//
void Lightning::init(double fromX, double fromY, double toX, double toY,
							char energy)
{
	x = fromX;
	y = fromY;
	destx = toX;
	desty = toY;
	energy_level = energy;
	v = 6.0;
	expect_steps = (int)( dist(desty-y, destx-x) / v * 1.2);
	if( expect_steps < 2 )
		expect_steps = 2;
	steps = 0;
	a0 = a = 8.0;
	r0 = r = 8.0 * a;
	wide = PI / 4;
	seed = (unsigned)(fromX + fromY + toX + toY) | 1;
	(void) rand_seed();
}
//-------- End of function Lightning::init ----------//

//-------- Begin of function Lightning::goal ----------//
//  return TRUE if the point is very near destination
int Lightning::goal()
{
	return( dist(destx-x, desty-y) < v );
}
//-------- End of function Lightning::goal ----------//

//-------- Begin of function Lightning::update_parameter ----------//
void Lightning::update_parameter()
{
	double progress = (double) steps / expect_steps;
	if( progress > 1)
		progress = 1;

	// a = a0;		// constant

	r = r0 * (1-progress);
	wide = 0.25 * ( 1 + progress ) * PI;
}
//-------- End of function Lightning::update_parameter ----------//

//-------- Begin of function Lightning::move_particle ----------//
void Lightning::move_particle()
{
	// determine attraction
	double attractionDist = dist(destx-x, desty-y);
	if( attractionDist < v)
		return;
	double aX = a * (destx-x) / attractionDist;
	double aY = a * (desty-y) / attractionDist;

	// determine random component
	double attractionAngle = atan2( desty-y, destx-x);
	double randomAngle = ((rand_seed() & 255)/128.0-1.0)*wide+ attractionAngle;
	double rX = r * cos(randomAngle);
	double rY = r * sin(randomAngle);

	// total
	double tX = aX + rX;
	double tY = aY + rY;
	double distt = dist(tX, tY);

	// move x and y, along tX, tY but the magnitude is v
	if( distt > 0)
	{
		x += v * tX / distt;
		y += v * tY / distt;
	}

	steps ++;
	update_parameter();
}
//-------- End of function Lightning::move_particle ----------//

//-------- Begin of function Lightning::draw_step ----------//
void Lightning::draw_step(VgaBuf *vgabuf)
{
	int prex, prey;
	if(!goal() )
	{
		prex = (int) x;
		prey = (int) y;
		move_particle();
		// BUGHERE: ignore if clipped, currently
		if( energy_level > 4)
		{
			if( prex >= bound_x1+2 && (int)x >= bound_x1+2 &&
				prex <= bound_x2-2 && (int)x <= bound_x2-2 &&
				prey >= bound_y1+2 && (int)y >= bound_y1+2 &&
				prey <= bound_y2-2 && (int)y <= bound_y2-2 )
			{
				vgabuf->line(prex+2, prey, (int) x+2, (int) y, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey+2, (int) x, (int) y+2, OUTLIGHTNCOLOR);
				vgabuf->line(prex-2, prey, (int) x-2, (int) y, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey-2, (int) x, (int) y-2, OUTLIGHTNCOLOR);
				vgabuf->line(prex+1, prey, (int) x+1, (int) y, INLIGHTNCOLOR);
				vgabuf->line(prex, prey+1, (int) x, (int) y+1, INLIGHTNCOLOR);
				vgabuf->line(prex-1, prey, (int) x-1, (int) y, INLIGHTNCOLOR);
				vgabuf->line(prex, prey-1, (int) x, (int) y-1, INLIGHTNCOLOR);
				vgabuf->line(prex, prey, (int) x, (int) y, CORELIGHTNCOLOR);
			}
		}
		else if( energy_level > 2)
		{
			if( prex >= bound_x1+1 && (int)x >= bound_x1+1 &&
				prex <= bound_x2-1 && (int)x <= bound_x2-1 &&
				prey >= bound_y1+1 && (int)y >= bound_y1+1 &&
				prey <= bound_y2-1 && (int)y <= bound_y2-1 )
			{
				vgabuf->line(prex+1, prey, (int) x+1, (int) y, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey+1, (int) x, (int) y+1, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey, (int) x, (int) y, INLIGHTNCOLOR);
			}
		}
		else
		{
			if( prex >= bound_x1 && (int)x >= bound_x1 &&
				prex <= bound_x2 && (int)x <= bound_x2 &&
				prey >= bound_y1 && (int)y >= bound_y1 &&
				prey <= bound_y2 && (int)y <= bound_y2 )
			{
				vgabuf->line(prex, prey, (int) x, (int) y, OUTLIGHTNCOLOR);
			}
		}

	}
}
//-------- End of function Lightning::draw_step ----------//

//-------- Begin of function Lightning::draw_whole ----------//
void Lightning::draw_whole(VgaBuf *vgabuf)
{
	int prex, prey;
	while(!goal() )
	{
		prex = (int) x;
		prey = (int) y;
		move_particle();
		// ignore clipping, currently
		if( energy_level > 4)
		{
			if( prex >= bound_x1+2 && (int)x >= bound_x1+2 &&
				prex <= bound_x2-2 && (int)x <= bound_x2-2 &&
				prey >= bound_y1+2 && (int)y >= bound_y1+2 &&
				prey <= bound_y2-2 && (int)y <= bound_y2-2 )
			{
				vgabuf->line(prex+2, prey, (int) x+2, (int) y, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey+2, (int) x, (int) y+2, OUTLIGHTNCOLOR);
				vgabuf->line(prex-2, prey, (int) x-2, (int) y, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey-2, (int) x, (int) y-2, OUTLIGHTNCOLOR);
				vgabuf->line(prex+1, prey, (int) x+1, (int) y, INLIGHTNCOLOR);
				vgabuf->line(prex, prey+1, (int) x, (int) y+1, INLIGHTNCOLOR);
				vgabuf->line(prex-1, prey, (int) x-1, (int) y, INLIGHTNCOLOR);
				vgabuf->line(prex, prey-1, (int) x, (int) y-1, INLIGHTNCOLOR);
				vgabuf->line(prex, prey, (int) x, (int) y, CORELIGHTNCOLOR);
			}
		}
		else if( energy_level > 2)
		{
			if( prex >= bound_x1+1 && (int)x >= bound_x1+1 &&
				prex <= bound_x2-1 && (int)x <= bound_x2-1 &&
				prey >= bound_y1+1 && (int)y >= bound_y1+1 &&
				prey <= bound_y2-1 && (int)y <= bound_y2-1 )
			{
				vgabuf->line(prex+1, prey, (int) x+1, (int) y, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey+1, (int) x, (int) y+1, OUTLIGHTNCOLOR);
				vgabuf->line(prex, prey, (int) x, (int) y, INLIGHTNCOLOR);
			}
		}
		else
		{
			if( prex >= bound_x1 && (int)x >= bound_x1 &&
				prex <= bound_x2 && (int)x <= bound_x2 &&
				prey >= bound_y1 && (int)y >= bound_y1 &&
				prey <= bound_y2 && (int)y <= bound_y2 )
			{
				vgabuf->line(prex, prey, (int) x, (int) y, OUTLIGHTNCOLOR);
			}
		}

	}
}
//-------- End of function Lightning::draw_whole ----------//

//-------- Begin of function Lightning::progress ----------//
double Lightning::progress()
{
	if(goal())
		return 1;
	else
		return (double) steps / expect_steps;
}
//-------- End of function Lightning::progress ----------//


//-------- Begin of function Lightning::draw_section ----------//
void Lightning::draw_section(VgaBuf *vgabuf, double portion)
{
	while( progress() < ( portion< 1.0 ? portion : 1.0 ) )
		draw_step(vgabuf);
}
//-------- End of function Lightning::draw_section ----------//


//-------- Begin of static function sqr ----------//
static double sqr(double x)
{
	return x*x;
}
//-------- End of static function sqr ----------//

