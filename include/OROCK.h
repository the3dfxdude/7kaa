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

// Filename    : OROCK.H
// Description : header file of Rock and RockArray
// Owner       : Gilbert

#ifndef __OROCK_H
#define __OROCK_H

#include <ODYNARRB.h>

// --------- define class Rock -----------//

#pragma pack(1)
class Rock
{
public:
	short	rock_recno;
	char	cur_frame;
	char	delay_remain;
	short loc_x;
	short loc_y;

private:
	unsigned seed;

public:
	Rock(short rockRecno, short xLoc, short yLoc);
	void	init(short rockRecno, short xLoc, short yLoc);	// cur_frame init to 1, and initDelay is random
	void	process();
	void	draw();
	void	draw_block(short xLoc, short yLoc);

private:
	unsigned random(unsigned);
};
#pragma pack()



// --------- define class RockArray -----------//

class RockArray : public DynArrayB
{
public:
	RockArray(int initArraySize = DEF_DYNARRAY_BLOCK_SIZE);
	void	init();
	void	deinit();

	void	process();
	int	add(Rock *r)				{ linkin(r); return recno();}
	void	del(int i)					{ linkout(i); }
	Rock* operator[](int n)			{ return (Rock *)get(n); }
};

extern RockArray rock_array;
extern RockArray dirt_array;

#endif
