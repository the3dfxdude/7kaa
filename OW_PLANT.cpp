// Filename   : OW_PLANT.CPP
// Description: growth of plant
// Ownership  : Gilbert

#include <OMATRIX.h>
#include <OWORLD.h>
#include <OWORLDMT.h>
#include <OWEATHER.h>
#include <OPLANT.h>
#include <OTERRAIN.h>
#include <ALL.h>
#include <math.h>


//------------ Define constant ---------------//
static short opt_temp[3] = { 32, 25, 28 };		// tropical,temperate and both
#define PLANT_ARRAY_SIZE 8

//------------ Define inline function -------//
inline int rand_inner_x()
{
	// can use m.random(ZOOM_LOC_WIDTH) instead
	// return (ZOOM_LOC_WIDTH *3)/8 + m.random(ZOOM_LOC_WIDTH/4);
	return ZOOM_LOC_WIDTH / 4 + m.random(ZOOM_LOC_WIDTH/2);
}

inline int rand_inner_y()
{
	return (ZOOM_LOC_HEIGHT * 3) / 8 + m.random(ZOOM_LOC_HEIGHT/4);
}

//----------- Begin of function World::plant_ops -----------//
//
void World::plant_ops()
{
	plant_grow(40);
	plant_reprod(10);
	plant_death();
	plant_spread(50);
}
//----------- End of function World::plant_ops -----------//

//------------ begin of function World::plant_grow ------------//
//
// pGrow = prabability of grow, range from 0 to 100
// scanDensity = scan one square per scanDensity^2
//
void World::plant_grow(int pGrow, int scanDensity)
{
	// scan part of the map for plant
	int yBase = m.random(scanDensity);
	int xBase = m.random(scanDensity);
	for( int y = yBase; y < max_y_loc; y += scanDensity)
		for( int x = xBase; x < max_x_loc; x += scanDensity)
		{
			Location *l = get_loc(x,y);
			short bitmapId, basePlantId;

			// is a plant and is not at maximum grade
			if( l->is_plant() && m.random(100) < pGrow &&
				(basePlantId = plant_res.plant_recno(bitmapId = l->plant_id())) != 0 &&
				bitmapId - plant_res[basePlantId]->first_bitmap < plant_res[basePlantId]->bitmap_count -1)
			{
				// increase the grade of plant
				l->grow_plant();
			}
		}
}
//------------ end of function World::plant_grow ------------//


//------------ begin of function World::plant_reprod --------//
//
// pReprod = prabability of reproduction, range from 0 to 20
// scanDensity = scan one square per scanDensity^2
//
void World::plant_reprod(int pReprod, int scanDensity)
{
	if( plant_count > plant_limit )
		return;
	if( 5 * plant_count < 4 * plant_limit )
		pReprod++;              // higher probability to grow

	// determine the rainful, temperature and sunlight
	short t = weather.temp_c();

	// scan the map for plant
	int yBase = m.random(scanDensity);
	int xBase = m.random(scanDensity);
	for( int y = yBase; y < max_y_loc; y += scanDensity)
	{
		for( int x = xBase; x < max_x_loc; x += scanDensity)
		{
			Location *l = get_loc(x,y);
			short bitmapId, basePlantId, plantGrade;
			// is a plant and grade > 3
			if( l->is_plant() && (basePlantId = plant_res.plant_recno(
				bitmapId = l->plant_id())) != 0 &&
				((plantGrade = bitmapId - plant_res[basePlantId]->first_bitmap) >= 3 ||
				 plantGrade == plant_res[basePlantId]->bitmap_count-1))
			{
				// find the optimal temperature for the plant
				short oTemp = opt_temp[plant_res[basePlantId]->climate_zone -1];
				short tempEffect = 5 - abs( oTemp - t);
				tempEffect = tempEffect > 0 ? tempEffect : 0;

				if( m.random(100) < tempEffect * pReprod)
				{
					// produce the same plant but grade 1,
					char trial = 2;
					Location *newl;
					while( trial --)
					{
						newl = NULL;
						switch(m.random(8))
						{
						case 0:		// north square
							if( y > 0)
								newl = get_loc(x,y-1);
							break;
						case 1:		// east square
							if( x < max_x_loc-1 )
								newl = get_loc(x+1,y);
							break;
						case 2:		// south square
							if( y < max_y_loc-1 )
								newl = get_loc(x,y+1);
							break;
						case 3:		// west square
							if( x > 0)
								newl = get_loc(x-1,y);
							break;
						case 4:		// north west square
							if( y > 0 && x > 0)
								newl = get_loc(x-1,y-1);
							break;
						case 5:		// north east square
							if( y > 0 && x < max_x_loc-1 )
								newl = get_loc(x+1,y-1);
							break;
						case 6:		// south east square
							if( y < max_y_loc-1 && x < max_x_loc-1)
								newl = get_loc(x+1,y+1);
							break;
						case 7:		// south west square
							if( y < max_y_loc-1 && x > 0)
								newl = get_loc(x-1,y+1);
							break;
						}

						char teraType;
						// #### begin Gilbert 6/3 #######//
						if( newl && newl->can_add_plant() &&
							(plant_res[basePlantId]->tera_type[0] == 
							 (teraType = terrain_res[newl->terrain_id]->average_type) ||
							 plant_res[basePlantId]->tera_type[1] == teraType ||
							 plant_res[basePlantId]->tera_type[2] == teraType) )
						// #### end Gilbert 6/3 #######//
						{
							newl->set_plant(plant_res[basePlantId]->first_bitmap
								, rand_inner_x(), rand_inner_y() );

							// ------- set flammability ---------
							newl->set_fire_src(100);
							plant_count++;
							//### begin alex 24/6 ###//
							//newl->set_power_off();
							//newl->power_nation_recno = 0;
							//set_surr_power_off(x, y);
							//#### end alex 24/6 ####//
							break;
						}
					}					
				}
			}
		}
	}
}
//------------ end of function World::plant_reprod --------//


//------------ begin of function World::plant_spread ------------//
//
// pSpread = probability of spreading, range from 0 to 1000
//
void World::plant_spread(int pSpread)
{
	if( plant_count > plant_limit)
		return;
	if( 5 * plant_count < 4 * plant_limit )
		pSpread += pSpread;

	if(m.random(1000) >= pSpread )
		return;

	// ------- determine temperature
	short t = weather.temp_c();

	// ------- randomly select a place to seed plant
	int y = 1+m.random(max_y_loc-2);
	int x = 1+m.random(max_x_loc-2);

	Location *l = get_loc(x,y);
	int build_flag = TRUE;
	char teraType = terrain_res[l->terrain_id]->average_type;

	// ------- all square around are the same terrain type and empty
	for( int y1 = y-1; y1 <= y+1; ++y1)
		for( int x1 = x-1; x1 <= x+1; ++x1)
		{
			l = get_loc(x1,y1);
			// #### begin Gilbert 6/3 #######//
			if( !l->can_add_plant() || terrain_res[l->terrain_id]->average_type != teraType)
				build_flag = FALSE;
			// #### end Gilbert 6/3 #######//
		}

	if( build_flag)
	{
		char climateZone = 0;
		short plantBitmap = 0;
		for( int retry=0; !climateZone && retry < 5; ++retry)
		{
			for( char j=0; j < 3; ++j)
			{
				if( m.random(5) > abs(t- opt_temp[j]) )
				{
					climateZone = j+1;
					plantBitmap = plant_res.scan( climateZone, teraType, 0);
					if( plantBitmap)
					{
						l = get_loc(x,y);
						l->set_plant( plantBitmap, rand_inner_x(), rand_inner_y() );
						l->set_fire_src(100);
						plant_count++;
						//### begin alex 24/6 ###//
						//l->set_power_off();
						//l->power_nation_recno = 0;
						//set_surr_power_off(x, y);
						//#### end alex 24/6 ####//
					}
					break;
				}
			}
		}
	}
}
//------------ end of function World::plant_spread ------------//


//------------ begin of function World::plant_death ---------//
//
// a plant may death, if it is surrounded by many trees
//
void World::plant_death(int scanDensity)
{
	int yBase = m.random(scanDensity);
	int xBase = m.random(scanDensity);
	for( int y = yBase; y < max_y_loc; y += scanDensity)
	{
		for( int x = xBase; x < max_x_loc; x += scanDensity)
		{
			Location *locPtr = get_loc(x,y);
			if( locPtr->is_plant() )
				{
				char neighbour =0;
				char totalSpace =0;

				// west
				if( x > 0)
				{
					totalSpace++;
					if( (locPtr-1)->is_plant() )
						neighbour++;
				}

				// east
				if( x < max_x_loc-1)
				{
					totalSpace++;
					if( (locPtr+1)->is_plant() )
						neighbour++;
				}

				if( y > 0)
				{
					locPtr = get_loc(x,y-1);

					// north square
					totalSpace++;
					if( locPtr->is_plant() )
						neighbour++;
				
					// north west
					if( x > 0)
					{
						totalSpace++;
						if( (locPtr-1)->is_plant() )
							neighbour++;
					}

					//	north east
					if( x < max_x_loc-1)
					{
						totalSpace++;
						if( (locPtr+1)->is_plant() )
							neighbour++;
					}
				}

				if( y < max_x_loc-1)
				{
					locPtr = get_loc(x,y+1);

					// south square
					totalSpace++;
					if( locPtr->is_plant() )
						neighbour++;
				
					// south west
					if( x > 0)
					{
						totalSpace++;
						if( (locPtr-1)->is_plant() )
							neighbour++;
					}

					// south east
					if( x < max_x_loc-1)
					{
						totalSpace++;
						if( (locPtr+1)->is_plant() )
							neighbour++;
					}
				}

				// may remove plant if more than two third of the space is occupied
				if( m.random(totalSpace) + 2*totalSpace/3 <= neighbour )
				{
					locPtr = get_loc(x,y);
					get_loc(x,y)->remove_plant();
					if( locPtr->fire_src() > 50)
						locPtr->set_fire_src(50);
					plant_count--;
					//### begin alex 24/6 ###//
					//newl->set_power_off();
					//newl->power_nation_recno = 0;
					//set_surr_power_off(x, y);
					//#### end alex 24/6 ####//
				}
			}
		}
	}
}
//------------ end of function World::plant_death ---------//


//------------ begin of function World::plant_init ------------//
// randomly select a place and call plant_spray to enlarge the
// forest
//
void World::plant_init()
{
	plant_count = 0;
	for(int trial = 50; trial > 0; --trial)
		{
		// ------- randomly select a place to seed plant
		int y = 1+m.random(max_y_loc-2);
		int x = 1+m.random(max_x_loc-2);

		Location *l = get_loc(x,y);
		int build_flag = TRUE;
		char teraType = terrain_res[l->terrain_id]->average_type;

		// ------- all square around are the same terrain type and empty
		for( int y1 = y-1; y1 <= y+1; ++y1)
			for( int x1 = x-1; x1 <= x+1; ++x1)
			{
				l = get_loc(x1,y1);
				// #### begin Gilbert 6/3 #######//
				if( !l->can_add_plant() || terrain_res[l->terrain_id]->average_type != teraType)
					build_flag = FALSE;
				// #### end Gilbert 6/3 #######//
			}

		if( build_flag )
		{
			short plantBitmap = plant_res.scan( 0, teraType, 0);
			short plantArray[PLANT_ARRAY_SIZE];
			for( int i = 0; i < PLANT_ARRAY_SIZE; ++i)
			{
				plantArray[i] = plant_res.plant_recno(plant_res.scan(0, teraType, 0));
			}
			if( plantArray[0] )
			{
				plant_spray(plantArray, 6+m.random(4), x, y);
			}
		}
	}

	plant_limit = plant_count * 3 / 2;

	// ------- kill some plant ----------//
	for(trial = 8; trial > 0; --trial)
	{
		plant_death(2);
	}
}
//------------ end of function World::plant_init ------------//

//------------ begin of function World::plant_spray ------------//
void World::plant_spray(short *plantArray, char strength, short x, short y)
{
	if( strength <= 0)
		return;

	//---------- if the space is empty put a plant on it ----------//
	Location *newl = get_loc(x, y);
	short basePlantId = plantArray[m.random(PLANT_ARRAY_SIZE)];
	short plantSize = m.random(plant_res[basePlantId]->bitmap_count);
	if( plantSize > strength)
		plantSize = strength;

	char teraType;
	if( newl && newl->can_add_plant() && 
		(plant_res[basePlantId]->tera_type[0] == 
		 (teraType = terrain_res[newl->terrain_id]->average_type) ||
		 plant_res[basePlantId]->tera_type[1] == teraType ||
		 plant_res[basePlantId]->tera_type[2] == teraType) )
	{
		newl->set_plant(plant_res[basePlantId]->first_bitmap +plantSize
			, rand_inner_x(), rand_inner_y() );
		newl->set_fire_src(100);
		plant_count++;
		//### begin alex 24/6 ###//
		//newl->set_power_off();
		//newl->power_nation_recno = 0;
		//set_surr_power_off(x, y);
		//#### end alex 24/6 ####//
	}
	else if( newl && newl->is_plant() &&
		// 1. same type, large override small
		// newl->plant_id() >= plant_res[basePlantId]->first_bitmap &&
		// newl->plant_id() < plant_res[basePlantId]->first_bitmap + plantSize)
		// 2. same type, small override large
		// newl->plant_id() > plant_res[basePlantId]->first_bitmap + plantSize &&
		// newl->plant_id() < plant_res[basePlantId]->first_bitmap + plant_res[basePlantId]->bitmap_count)
		// 3. all types, small override large
		(newl->plant_id() - plant_res[plant_res.plant_recno(newl->plant_id())]->first_bitmap) >
		plantSize )
	{
		// same kind of plant, but smaller, override by a smaller one
		newl->remove_plant();
		newl->set_plant(plant_res[basePlantId]->first_bitmap +plantSize
			, rand_inner_x(), rand_inner_y() );
		newl->set_fire_src(100);
		//### begin alex 24/6 ###//
		//newl->set_power_off();
		//newl->power_nation_recno = 0;
		//set_surr_power_off(x, y);
		//#### end alex 24/6 ####//
	}
	else
	{
		plantSize = -1;
	}

	if( plantSize >= 0 && strength)
	{
		char trial = 3;
		while( trial--)
		{
			switch(m.random(8))
			{
			case 0:		// north square
				if( y > 0)
					plant_spray(plantArray, strength-1, x,y-1);
				break;
			case 1:		// east square
				if( x < max_x_loc-1 )
					plant_spray(plantArray, strength-1, x+1,y);
				break;
			case 2:		// south square
				if( y < max_y_loc-1 )
					plant_spray(plantArray, strength-1, x,y+1);
				break;
			case 3:		// west square
				if( x > 0)
					plant_spray(plantArray, strength-1, x-1,y);
				break;
			case 4:		// north west square
				if( y > 0 && x > 0)
					plant_spray(plantArray, strength-1, x-1,y-1);
				break;
			case 5:		// north east square
				if( y > 0 && x < max_x_loc-1 )
					plant_spray(plantArray, strength-1, x+1,y-1);
				break;
			case 6:		// south east square
				if( y < max_y_loc-1 && x < max_x_loc-1)
					plant_spray(plantArray, strength-1, x+1,y+1);
				break;
			case 7:		// south west square
				if( y < max_y_loc-1 && x > 0)
					plant_spray(plantArray, strength-1, x-1,y+1);
				break;
			}
		}
	}
}
//------------ end of function World::plant_spray ------------//

