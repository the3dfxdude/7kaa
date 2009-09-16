// Filename    : OSNOW.H
// Description : header file for class SnowLayer
// Ownership   : Gilbert

#ifndef __OSNOW_H
#define __OSNOW_H

//------------ Define constant -------------//

#define SNOW_LAYERS 4

//--------- Define class SnowLayer ----------//

class VgaBuf;

class SnowLayer
{
public:
	int	bound_x1, bound_y1, bound_x2, bound_y2;
	short	h_sep, v_sep;			// horizontal/vertical separation of each snow
	short	fall_speed;				// falling speed
	short	amplitude;				// amplitude of random shift
	short	radius;					// radius of each snow
	short slide_speed;			// horizontal speed
	short period;					// no. of row to draw, maximum 1000
	char	blind_site;
	char	anim_speed;				// animation speed range from 1 to 10
	char	anim_phase;

private:
	int	snow_x, snow_y;		// location of any snow;
	unsigned seed;
	unsigned random(unsigned);
	
public:
	void	set_bound(int x1, int y1, int x2, int y2);
	void	init(short h, short v, short speed, short amp, short r, double s,
			char animSpeed = 100, short initPeriod = 1000);


	void	fall();
	void	draw_step(VgaBuf *);

};

//----------- Define class Snow ------------//

class Snow
{
public:
	SnowLayer layer[SNOW_LAYERS];

public:
	void	set_bound(int x1, int y1, int x2, int y2);
	void	init(double slope, char animSpeed);				// animSpeed from 1 to 10
	void	fall();
	void	draw_step(VgaBuf *vgabuf);
};

//------------------------------------------//

#endif