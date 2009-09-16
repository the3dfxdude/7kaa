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