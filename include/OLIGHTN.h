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

// Filename    : OLIGHTN.H
// Description : Header file for Lightning class
// Ownership   : Gilbert

#ifndef __LIGHTN_H
#define __LIGHTN_H

//----------- Define constant -----------//

#define MAX_BRANCH 8

//--------- Define class Lightning ----------//

class VgaBuf;

class Lightning
{
public:
	static int bound_x1, bound_y1, bound_x2, bound_y2;

	double x,y;					// particle coordinate
	double destx,desty;		// destination coordinate

	int	steps;				// no. of step
	int	expect_steps;		// expected no. of steps
	double v;					// magnitude of movement of x,y
	double a, a0;				// magnitude of ax, ay
	double r, r0;				// magnitude of rx, ry
	double wide;				// radian, MAX of angle of random vector
	unsigned	seed;				// last random number
	char	energy_level;		// initially 8

public:
	virtual ~Lightning();

	static double 	dist(double dx, double dy);
	static void 	set_clip(int x1, int y1, int x2, int y2);
				int	goal();
	virtual void	init(double fromX, double fromY, double toX, double toY, char energy);
	virtual void	update_parameter();
	virtual void	move_particle();
	virtual void	draw_step(VgaBuf *);		// move_particle and draw the line segment
	virtual void	draw_whole(VgaBuf *);	// draw the whole lightning on VgaBuf
			  double	progress();					// return 0.0 to 1.0
	virtual void	draw_section(VgaBuf *, double portion);

protected:
	unsigned rand_seed();			// shuffle and return seed
};

//--------- Define class YLightning ----------//

class YLightning : public Lightning
{
public:
	Lightning* 	branch[MAX_BRANCH];
	int			used_branch;
	unsigned		branch_prob;		// probability * 1000;
	int			branch_left;

public:
	YLightning();
	~YLightning();

	void init(double fromX, double fromY, double toX, double toY, char energy);
	void move_particle();
	void draw_step(VgaBuf *);			// move_particle and draw a line segment
	void draw_whole(VgaBuf *);
};

//--------------------------------------------//

#endif

