// Filename    : ORAIN2.CPP
// Description : class Rain
// Ownership   : Gilbert

#include <ORAIN.H>
#include <OVGABUF.H>


//------- Begin of function Rain::start_rain --------------//
//
// short density = no. of drop created per turn (usually 2 to 12)
// double slop = slop of the rain drop, 0.0 if no wind (fall vertically)
//               +ve for right, -ve for left.
void Rain::start_rain(int x1, int y1, int x2, int y2, short density, double slope)
{
	bound_x1 = x1;
	bound_y1 = y1;
	bound_x2 = x2;
	bound_y2 = y2;
	wind_slope = slope;
	drop_per_turn = density;
	clear();
	last_drop = -1;
}
//------- End of function Rain::start_rain --------------//

//------- Begin of function Rain::clear --------------//
//
void Rain::clear()
{
	for(int i = 0; i < MAX_RAINDROP; ++i)
	{
		drop_flag[i] = 0;
		spot_flag[i] = 0;
	}
	active_drop = 0;
	active_spot = 0;
}
//------- End of function Rain::clear --------------//


//------- Begin of function Rain::stop_rain --------------//
//
void Rain::stop_rain()
{
	drop_per_turn = 0;
}
//------- End of function Rain::stop_rain --------------//


//------- Begin of function Rain::new_drops --------------//
//
void Rain::new_drops()
{
	short dropRemain = drop_per_turn;
	short maxScan = MAX_RAINDROP;
	short i = last_drop;
	while(dropRemain > 0 && maxScan-- > 0)
	{
		i = (i + 1) % MAX_RAINDROP;
		if( !drop_flag[i])
		{
			short fromX = bound_x1+rand_seed()%(bound_x2-bound_x1);
			short height = (bound_y2-bound_y1)/8 + rand_seed()%(((bound_y2-bound_y1)*7)/8);
			short speed = height / 4;
			drop[i].init(this, fromX, (short)bound_y1, short(fromX+height*wind_slope),
				short(bound_y1+height), speed );
			drop_flag[i] = 1;
			active_drop++;
			dropRemain--;
			last_drop = i;
		}
	}
}
//------- End of function Rain::new_drops --------------//


//------- Begin of function Rain::new_spot ---------//
void Rain::new_spot(short x, short y)
{
	short spotRemain = 1;
	short maxScan = MAX_RAINDROP;
	short i = last_spot;
	while(spotRemain > 0 && maxScan-- > 0)
	{
		i = (i + 1) % MAX_RAINDROP;
		if( !spot_flag[i])
		{
			spot[i].init(this, x,y, drop_per_turn>4 ? 6:4);
			spot_flag[i] = 1;
			active_spot++;
			spotRemain--;
			last_spot = i;
		}
	}
}
//------- End of function Rain::new_spot ---------//


//------- Begin of function Rain::draw_step --------------//
// note : call Rain::new_drops before Rain::draw_step
//
void Rain::draw_step(VgaBuf *vgabuf)
{
	// --------- draw rain spot ------- //
	for(int i = 0; i < MAX_RAINDROP; ++i)
	{
		if( spot_flag[i])
		{
			spot[i].draw_step(vgabuf);
			spot_flag[i] = !spot[i].is_goal();
		}
	}

	// --------- draw rain drop ------- //
	for(i = 0; i < MAX_RAINDROP; ++i)
	{
		if(drop_flag[i])
		{
			drop[i].draw_step(vgabuf);
			if( drop[i].is_goal() )
			{
				new_spot(drop[i].dest_x, drop[i].dest_y);
				drop_flag[i] = 0;
			}
		}
	}
}
//------- End of function Rain::draw_step --------------//


//------- Begin of function Rain::is_all_clear --------------//
//
int Rain::is_all_clear()
{
	int count = 0;
	for(int i = 0; i < MAX_RAINDROP; ++i)
		if(drop_flag[i])
			count++;
	// update active_drop;
	active_drop = count;

	int count2 = 0;
	for(i = 0; i < MAX_RAINDROP; ++i)
		if(spot_flag[i])
			count2++;
	// update active_spot;
	active_spot = count2;
	return (count+count2 > 0);
}
//------- End of function Rain::is_all_clear --------------//


//------- Begin of function Rain::rand_seed --------------//
//
unsigned Rain::rand_seed()
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed;
}
//------- End of function Rain::rand_seed --------------//
