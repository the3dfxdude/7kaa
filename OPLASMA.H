// Filename    : OPLASMA.H
// Description : header file of class Plasma
// Ownership   : Gilbert

#ifndef __OPLASMA_H
#define __OPLASMA_H

typedef unsigned char BYTE;
typedef unsigned short U16;
typedef signed long S32;

// -------- Define Class Plasma --------//

class Plasma
{
public:
	short max_x;				// no. of column
	short max_y;				// no. of row
	short *matrix;				// 2D matrix of (max_x+1)*(max_y+1)
	int iparmx;					// iparmx = parm.x * 16
	int recur1;
	int recur_level;


public:
	Plasma();
	~Plasma();
	void	init(short x, short y);
	void	deinit();
	void	generate(int genMethod, int grainFactor, int randomSeed);
	void	generate2(int genMethod, int grainFactor, int randomSeed);

	short get_pix(short x, short y);
	void	plot(short x, short y, short value);
	void	add_base_level(short baseLevel);
	int	calc_tera_base_level(short minHeight);
	int	stat(int groups, short *minHeights, int *freq);

	void	shuffle_level(short minHeight, short maxHeight, short amplitude);

private:
	void	sub_divide(int x1, int y1, int x2, int y2);
	int	new_sub_divide(int x1,int y1,int x2,int y2, int recur);
	U16	adjust(int xa,int ya,int x,int y,int xb,int yb);


};

#endif
