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

// Filename    : OSNOWG.CPP
// Description : the bitmap of a snow ground bitmap
// Owner       : Gilbert

#include <OSNOWG.h>
#include <OSNOWRES.h>
#include <OWEATHER.h>
#include <OWORLDMT.h>

// ------- Define constant --------//
#define SCAN_DENSITY 4
#define INC_SNOW_CONSTANT 8
#define INC_SNOW_RATE 8
#define MAX_SNOW_THICKNESS 0x1800
#define DEC_SNOW_RATE 10
#define SNOW_GRADE_FACTOR 0x200

// ------- Begin of function SnowGround::init --------//
//
// initialize a snowGround
// <short> absX,absY		absolute coordinate (pixels on the map)
// [short] snowMapId		snow bitmap id, (default : choose randomly)
//
void SnowGround::init(short absX, short absY, int snowMapId)
{
	abs_x = absX;
	abs_y = absY;
	snow_map_id = snowMapId ? snowMapId : snow_res.rand_root(misc.random(1000));
	minor_hp = 0;
	SnowInfo *snowInfo = snow_res[snow_map_id];
	abs_x1 = abs_x + snowInfo->offset_x;
	abs_y1 = abs_y + snowInfo->offset_y;
	abs_x2 = abs_x1 + snowInfo->bitmap_width()-1;
	abs_y2 = abs_y1 + snowInfo->bitmap_height()-1;
}
// ------- End of function SnowGround::init --------//


// ------- Begin of function SnowGround::grow --------//
void SnowGround::grow(int rate)
{
	minor_hp += rate;
	if( minor_hp > 100)
	{
		minor_hp -= 100;
		snow_map_id = snow_res[snow_map_id]->rand_next(misc.random(1000));
		SnowInfo *snowInfo = snow_res[snow_map_id];
		abs_x1 = abs_x + snowInfo->offset_x;
		abs_x1 = abs_y + snowInfo->offset_y;
		abs_x2 = abs_x1 + snowInfo->bitmap_width()-1;
		abs_y2 = abs_y1 + snowInfo->bitmap_height()-1;
	}
}
// ------- End of function SnowGround::grow --------//


// ------- Begin of function SnowGround::dying --------//
int SnowGround::dying(int rate)
{
	minor_hp -= rate;
	if( minor_hp < 0)
	{
		if( snow_res[snow_map_id]->is_root() )
		{
			snow_map_id = 0;
			return 1;
		}
		else
		{
			minor_hp += 100;
			snow_map_id = snow_res[snow_map_id]->rand_prev(misc.random(1000));
			SnowInfo *snowInfo = snow_res[snow_map_id];
			abs_x1 = abs_x + snowInfo->offset_x;
			abs_x1 = abs_y + snowInfo->offset_y;
			abs_x2 = abs_x1 + snowInfo->bitmap_width()-1;
			abs_y2 = abs_y1 + snowInfo->bitmap_height()-1;
		}
	}
	return 0;
}
// ------- End of function SnowGround::dying --------//


// ------- Begin of function SnowGroundArray::init ------//
void SnowGroundArray::init(long initSnowScale, long anyNumber)
{
	snow_thick = initSnowScale * MAX_SNOW_THICKNESS / 8;
	snow_pattern = (anyNumber & 255) + 1;
}
// ------- End of function SnowGroundArray::init ------//


// ------ Begin of function SnowGroundArray::inc_snow -----//
void SnowGroundArray::inc_snow(int snowScale)
{
	if( snowScale > 0)
	{
		snow_thick += (snowScale + INC_SNOW_CONSTANT ) * INC_SNOW_RATE;
		if( snow_thick > MAX_SNOW_THICKNESS )
			snow_thick = MAX_SNOW_THICKNESS;
	}
}
// ------ End of function SnowGroundArray::inc_snow -----//


// ------ Begin of function SnowGroundArray::dec_snow -----//
void SnowGroundArray::dec_snow(int decRate)
{
	decRate *= DEC_SNOW_RATE;
	if( snow_thick > 0)
	{
		if( snow_thick <= decRate )
		{
			snow_thick = 0;
			snow_pattern = (snow_pattern & 255) + 1;
		}
		else
		{
			snow_thick -= decRate;
		}
	}
}
// ------ End of function SnowGroundArray::dec_snow -----//


// ------ Begin of function SnowGroundArray::process -----//
void SnowGroundArray::process()
{
	int snowScale = weather.snow_scale();
	if( snowScale > 0)
	{
		inc_snow(snowScale);
	}
	else
	{
		dec_snow(weather.temp_c() );
	}
}
// ------ End of function SnowGroundArray::process -----//


// ------ Begin of function SnowGroundArray::has_snow -------//
int SnowGroundArray::has_snow(short x, short y)
{
	seed = (snow_pattern << 16) + (x+y+257)*(5*x+y+1);
	(void) rand_seed();
	long height = (rand_seed() & 0xffff) - (0xffff - snow_thick);
	if( height <= 0)
		return 0;

	int snowGrade = height / SNOW_GRADE_FACTOR;
	SnowInfo *snowInfo = snow_res[snow_res.rand_root(rand_seed()/4)];
	for( ; snowGrade > 0; --snowGrade)
	{
		snowInfo = snowInfo->rand_next_ptr(rand_seed()/4);
	}
	return snowInfo->snow_map_id;
}
// ------ End of function SnowGroundArray::has_snow -------//


// ------ Begin of function SnowGroundArray::random -------//
unsigned SnowGroundArray::rand_seed()
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed;
}
// ------ End of function SnowGroundArray::random -------//
