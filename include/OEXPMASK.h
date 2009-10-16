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

// Filename    : OEXPMASK.H
// Description : Header file of explored area mask
// Owner       : Gilbert

#ifndef __OEXPMASK_H
#define __OEXPMASK_H

struct Location;
class ColorTable;

class ExploredMask
{
public:
	char *mask_bitmap;
	char *remap_bitmap;
	ColorTable *brightness_table;

public:
	void init(ColorTable *);
	void deinit();

	void draw(short xLoc, short yLoc, int northRow, int thisRow, int southRow);
};

extern ExploredMask explored_mask;

#endif