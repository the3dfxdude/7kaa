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

// Filename    : OSNOW2.CPP
// Description : Class Snow
// Ownership   : Gilbert

#include <OSNOW.h>
#include <OVGABUF.h>
#include <ALL.h>

//------------ Begin of function Snow::set_bound ---------//
void Snow::set_bound(int x1, int y1, int x2, int y2)
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		layer[i].set_bound(x1, y1, x2, y2);
	}
}
//------------ End of function Snow::set_bound ---------//

//------------ Begin of function Snow::init ---------//
void Snow::init(double s, char animSpeed)
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		// slower the snow, denser
		layer[i].init( 15 + 10*i + animSpeed, 20 + 10*i + animSpeed, 3+i*2, i+2, i/2, s, animSpeed);
	}
}
//------------ End of function Snow::init ---------//

//------------ Begin of function Snow::fall ---------//
void Snow::fall()
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		layer[i].fall();
	}
}
//------------ End of function Snow::fall ---------//

//------------ Begin of function Snow::draw_step ---------//
void Snow::draw_step(VgaBuf *vgabuf)
{
	for(int i = 0; i < SNOW_LAYERS; ++i)
	{
		layer[i].draw_step(vgabuf);
	}
}
//------------ End of function Snow::draw_step ---------//
