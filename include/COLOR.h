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

//Filename    : COLOR.H
//Description : Game Color Definition

#ifndef __COLOR_H
#define __COLOR_H

//--------- Define color codes ----------//

#define VGA_RED 					0xA0
#define VGA_LIGHT_BLUE			0xA4
#define VGA_DARK_BLUE			0xC4
#define VGA_LIGHT_GREEN			0xA8
#define VGA_DARK_GREEN			0xC8
#define VGA_PURPLE				0xAC
#define VGA_ORANGE				0xD0
#define VGA_YELLOW				0xB4
#define VGA_BROWN					0xB8
#define VGA_GRAY					0x90

#define V_BLACK              	0x00       // single color only
#define V_WHITE              	0x9F
#define V_RED                	VGA_RED+1
#define V_LIGHT_BLUE				VGA_LIGHT_BLUE+1
#define V_DARK_BLUE				VGA_DARK_BLUE+1
#define V_LIGHT_GREEN         VGA_LIGHT_GREEN+1
#define V_DARK_GREEN          VGA_DARK_GREEN+3
#define V_PURPLE					VGA_PURPLE+1
#define V_ORANGE					VGA_ORANGE
#define V_YELLOW					VGA_YELLOW
#define V_BROWN					VGA_BROWN+2

#define V_BACKGROUND         	0xFF               // background color, pixels of this color are not put in VGAputIcon

//---------- Define Game Colors --------------//

#define OWN_SELECT_FRAME_COLOR		V_YELLOW
#define ENEMY_SELECT_FRAME_COLOR		V_RED

#define UNEXPLORED_COLOR				V_BLACK

#define SITE_COLOR						V_BLACK
#define WALL_COLOR						VGA_GRAY+6
#define HILL_COLOR						VGA_BROWN+3
#define FIRE_COLOR						V_RED
#define TORNADO_COLOR1					VGA_GRAY+7
#define TORNADO_COLOR2					VGA_GRAY+8

#define INDEPENDENT_NATION_COLOR		V_WHITE

//---------------------------------------------//

#endif
