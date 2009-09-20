// Filename    : OANLINE.H
// Description : header file for Animated Line, AnimLine
// Ownership   : Gilbert


#ifndef __OANLINE_H
#define __OANLINE_H

class VgaBuf;

class AnimLine
{
	enum 
	{
		ANIMLINE_PERIOD = 8,
		ANIMCOLOR_PERIOD = 6,
		ANIMCOLOR_INNER_PERIOD = 2,
		ANIMCOLOR_SERIES = 3,
	};

public:
	short		bound_x1;
	short		bound_y1;
	short		bound_x2;
	short		bound_y2;
	short		phase;
	short		color_phase;

	static	unsigned char init_color_code[ANIMCOLOR_PERIOD][ANIMLINE_PERIOD];
	static	unsigned char series_color_code[ANIMCOLOR_SERIES][ANIMLINE_PERIOD];

public:
	void		init( short x1, short y1, short x2, short y2);
	void		draw_line( VgaBuf *, short x1, short y1, short x2, short y2, int animatedFlag=1, int effectFlag=0);
	void		thick_line( VgaBuf *, short x1, short y1, short x2, short y2, int animatedFlag=1, int effectFlag=0);
	void		inc_phase();

	void		basic_line( VgaBuf *, short x1, short y1, short x2, short y2, int animatedFlag, unsigned char* colorCode);
	void		basic_hline( VgaBuf *, short x1, short x2, short y1, int animatedFlag, unsigned char* colorCode);
	void		basic_vline( VgaBuf *, short x1, short y1, short y2, int animatedFlag, unsigned char* colorCode);

	unsigned char *get_series_color_array(int effectFlag);

private:
	short		top_intercept( short x1, short y1, short x2, short y2);
	short		bottom_intercept( short x, short y, short x2, short y2);
	short		left_intercept( short x, short y, short x2, short y2);
	short		right_intercept( short x, short y, short x2, short y2);
};

extern AnimLine anim_line;

#endif