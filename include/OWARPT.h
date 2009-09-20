// Filename    : OWARPT.H
// Description : warning point on map


#ifndef __OWARPT_H
#define __OWARPT_H

#include <OWORLDMT.h>

struct WarPoint
{
	DWORD strength;

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