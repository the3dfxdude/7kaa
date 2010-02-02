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

// Filename   : ORAIN3.CPP
// Description: class rain spot
// Ownership  : Gilbert

#include <ORAIN.h>
#include <OVGABUF.h>
#include <COLOR.h>
#include <OWORLDMT.h>


// -------- Begin of function RainSpot::init -------//
void RainSpot::init(Rain *, short destX, short destY, short maxStep)
{
	// rain_ptr = rain;
	center_x = destX;
	center_y = destY;
	step = 0;
	max_step = maxStep;
}
// -------- End of function RainSpot::init -------//


// -------- Begin of function RainSpot::fall -------//
void RainSpot::fall()
{
	step ++;
}
// -------- End of function RainSpot::fall -------//


// -------- Begin of function RainSpot::draw_step -------//
void RainSpot::draw_step(VgaBuf *vgabuf)
{
	fall();

	short x,y;
	/*
	x = center_x - step;
	y = center_y - step/2;
	vgabuf->draw_pixel(x,y,0x73);
	x = center_x - step/2;
	y = center_y - step;
	vgabuf->draw_pixel(x,y,0x73);
	x = center_x + step/2;
	y = center_y - step;
	vgabuf->draw_pixel(x,y,0x73);
	x = center_x + step;
	y = center_y - step/2;
	vgabuf->draw_pixel(x,y,0x73);
	*/
	// BUGHERE : didn't check ZOOM_X1,Y1,X2,Y2
	x = center_x - step;
	y = center_y - step;
	// ##### begin Gilbert 8/5 ######//
	if( x >= ZOOM_X1 && x < ZOOM_X2 && y >= ZOOM_Y1 && y < ZOOM_Y2)
		vgabuf->draw_pixel(x,y,VGA_GRAY+12);
	x = center_x - step;
	y = center_y + step/2;
	if( x >= ZOOM_X1 && x < ZOOM_X2 && y >= ZOOM_Y1 && y < ZOOM_Y2)
		vgabuf->draw_pixel(x,y,VGA_GRAY+12);
	x = center_x + step;
	y = center_y - step;
	if( x >= ZOOM_X1 && x < ZOOM_X2 && y >= ZOOM_Y1 && y < ZOOM_Y2)
		vgabuf->draw_pixel(x,y,VGA_GRAY+12);
	x = center_x + step;
	y = center_y + step/2;
	if( x >= ZOOM_X1 && x < ZOOM_X2 && y >= ZOOM_Y1 && y < ZOOM_Y2)
		vgabuf->draw_pixel(x,y,VGA_GRAY+12);
	// ##### end Gilbert 8/5 ######//
}
// -------- End of function RainSpot::draw_step -------//


// -------- Begin of function RainSpot::is_goal -------//
int RainSpot::is_goal()
{
	return step > max_step;
}
// -------- End of function RainSpot::is_goal -------//
