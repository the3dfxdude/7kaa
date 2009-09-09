// Filename   : ORAIN.H
// Description: Header file of class Rain & RainDrop
// Ownership   : Gilbert

#ifndef __ORAIN_H
#define __ORAIN_H

//------------ Define constant -------------//

#define MAX_RAINDROP 50

class VgaBuf;
class Rain;

//--------- Define class RainDrop ----------------//

class RainDrop
{
public:
	// Rain  *rain_ptr;
	short cur_x, cur_y;
	short dest_x, dest_y;
	short fall_speed;

public:
	void	init(Rain *rain, short fromX, short fromY, short toX, short toY, short speed );
	void	fall();
	void	draw_step(VgaBuf *);
	int	is_goal();
};

// -------- Define class RainSpot ---------//

class RainSpot
{
public:
	// Rain	*rain_ptr;
	short	center_x, center_y;
	short	step;
	short max_step;

public:
	void	init(Rain *rain, short destX, short destY, short maxStep);
	void	fall();
	void	draw_step(VgaBuf *);
	int	is_goal();
};

//--------- Define class Rain -------------//

class Rain
{
public:
	short		bound_x1, bound_x2, bound_y1, bound_y2;
	double	wind_slope;
	short		drop_per_turn;

private:
	RainDrop drop[MAX_RAINDROP];
	char		drop_flag[MAX_RAINDROP];
	short		active_drop;			// no. of active drops
	short		last_drop;

	RainSpot	spot[MAX_RAINDROP];
	char		spot_flag[MAX_RAINDROP];
	short		active_spot;			// no. of active drops
	short		last_spot;
	unsigned	seed;

public:
	void	start_rain(int x1, int y1, int x2, int y2, short density, double slope);
	void	clear();						// remove all rain drops
	void	stop_rain();					// no more new rain drops
	void	new_drops();
	void	new_spot(short x, short y);
	void	draw_step(VgaBuf *);

	int	is_all_clear();				// any raindrop is active?
	int	is_raining()			{ return active_drop>0 || active_spot > 0; }

private:
	unsigned	rand_seed();
};
//-------------------------------------------------//

#endif
