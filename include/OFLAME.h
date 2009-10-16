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

// Filename    : OFLAME.H
// Description : header file of class Flame
// Ownership   : Gilbert

#ifndef __OFLAME_H
#define __OFLAME_H

//---------- define constant -------//
#define FLAME_GROW_STEP 4

enum FlameType
{
	FLAME_CENTRE_POINT,
	FLAME_RANDOM_POINTS,
	FLAME_WIDE
};

//-------- define class Flame ----------//

class VgaBuf;

class Flame
{
public:
	unsigned char* heat_map;				// array of size map_width * map_height
	unsigned char* bitmap;					// array of size map_width * map_height +4
													// to be drawn by IMGbltTrans and IMGbltAreaTrans
	unsigned seed;
	short	map_width;
	short map_height;
	short decay;
	short smooth;
	short shade_base;							// init -1, changed after gen_bitmap

	// parameter for init and displaying flame[0], flame[1]...
public:
	static int grade(int flameStr)				{ return (FLAME_GROW_STEP * flameStr)/(20+ flameStr); }
	static int default_width(int flameGrade)	{ return flameGrade*8+40; }
	static int default_height(int flameGrade)	{ return flameGrade*12+28; }
	static int offset_x(int flameGrade)			{ return -4-4*flameGrade; }	// (ZOOM_LOC_HEIGHT - Flame::default_width)/2
	static int offset_y(int flameGrade)			{ return 4-flameGrade*12; }	// ZOOM_LOC_WIDTH - Flame::default_height
	static int base_width(int flameGrade)		{ return flameGrade *2 +4; }

public:
	Flame();
	Flame(short width, short height, short flameWidth, FlameType);
	~Flame();

	void init(short width, short height, short flameWidth, FlameType);
	void deinit();

	Flame& operator= (Flame &);
	void heat_up(short);
	void rise(short wind);				// -1 to +1
	void gen_bitmap(unsigned char shadeColor = 0xb4);
	void mask_bottom();
	void mask_transparent();
	void draw_step(short left, short bottom, VgaBuf *vgabuf, short wind); // coordinate of left and bottom
	void flush_point(short x=-1, short y=-1);

private:
	unsigned random(unsigned);

};

extern Flame flame[FLAME_GROW_STEP];

#endif