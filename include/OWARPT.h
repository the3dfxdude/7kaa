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

// Filename    : OWARPT.H
// Description : warning point on map


#ifndef __OWARPT_H
#define __OWARPT_H

#include <OWORLDMT.h>

struct WarPoint
{
	int strength;

	void	inc();
	void	decay();
};


class WarPointArray 
{
public:
	WarPoint *war_point;
	char		init_flag;
	char		draw_phase;

	WarPointArray();
	~WarPointArray();

	void	init();
	void	deinit();

	void	draw_dot();
	void	process();

	WarPoint *get_ptr(int xLoc, int yLoc);
	void	add_point(int xLoc, int yLoc);
};

extern WarPointArray war_point_array;

#endif