// Filename   : SnowGround
// Description: Header file of Snow Ground
// Owner      : Gilbert

#ifndef __SNOWG_H
#define __SNOWG_H

#include <ODYNARRB.H>

class SnowGround
{
public:
	int	snow_map_id;	// 0 for non-existing
	int	minor_hp;		// 0 to 99, promote if >=100
	short	abs_x;
	short	abs_y;
	short abs_x1;
	short abs_y1;
	short abs_x2;
	short abs_y2;

public:
	void	init(short absX, short absY, int snowMapId=0);
	void	grow(int rate);
	int	dying(int rate);			// 1 when die
	int	is_valid()				{ return snow_map_id; }
};

/*
#define SNOW_ZONE_SIZE 20
#if (SNOW_ZONE_SIZE < ZOOM_WIDTH / ZOOM_LOC_WIDTH || SNOW_ZONE_SIZE < ZOOM_HEIGHT / ZOOM_LOC_HEIGHT)
#error
#endif

#define WIDTH_IN_ZONE ((MAX_MAP_WIDTH+SNOW_ZONE_SIZE-1)/SNOW_ZONE_SIZE)
#define HEIGHT_IN_ZONE ((MAX_MAP_HEIGHT+SNOW_ZONE_SIZE-1)/SNOW_ZONE_SIZE)
*/

class SnowGroundArray
{
private:
	unsigned	seed;

public:
	long	snow_thick;
	long	snow_pattern;

public:
	void	init(long initSnowScale, long anyNumber);
	void	inc_snow(int snowScale);			// pass weather.snow_scale()
	void	dec_snow(int decRate);				// pass weather.temp_c()
	void	process();
	int	has_snow(short x, short y);		// return 0 or snow map id

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

private:
	unsigned rand_seed();
};

extern SnowGroundArray snow_ground_array;

#endif
