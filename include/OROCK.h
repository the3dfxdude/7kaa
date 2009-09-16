// Filename    : OROCK.H
// Description : header file of Rock and RockArray
// Owner       : Gilbert

#ifndef __OROCK_H
#define __OROCK_H

#include <ODYNARRB.H>

// --------- define class Rock -----------//

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