// Filename   : OW_FIRE.CPP
// Description: World class function related to spreading of fire
// Ownership  : Gilbert


#include <OWORLD.H>
#include <OMATRIX.H>
#include <OWEATHER.H>
#include <OWORLDMT.H>
#include <OTERRAIN.H>
#include <OUNIT.H>
#include <OFIRM.H>
#include <OFIRMA.H>
#include	<OCONFIG.H>
// #### begin Gilbert 29/5 #######//
#include <OSERES.H>
// #### end Gilbert 29/5 #######//
#include <math.h>
//### begin alex 6/8 ###//
#ifdef DEBUG
#include <OSYS.H>
#endif
//#### end alex 6/8 ####//


//------------ define constant ------------
#define SPREAD_RATE (config.fire_spread_rate)
// #define CONSUMPTION_RATE 1.0
#define WIND_SPREADFIRE (config.wind_spread_fire_rate)
#define FIRE_FADE_RATE (config.fire_fade_rate)

// 0 - 100
#define RESTORE_RATE (config.fire_restore_prob)

// 0 - 20
#define RAIN_SNOW_REDUCTION (config.rain_reduce_fire_rate)


// ----------- Define static function ----------//
static char bound_zero(char n)
{
	if( n > 0)
		return n;
	return 0;
}

// ---------- Level of Location::fire_str() ---------//
//   51 to  100	Highly flammable, restorable
//    1 to   50	flammable, restorable
//  -30 to    0   flammable but the fire cannot grow, or spreaded, restorable
//  -50 to  -31   flammable but the fire cannot grow, flammability unrestorable
// -100 to  -51   absolutely non-inflammable

// ----------- begin of function World::init_fire ---------- //
void World::init_fire()
{
	Location *locPtr = get_loc(0,0);
	for( int c = max_x_loc*max_y_loc; c >0; --c, ++locPtr)
	{
		if( locPtr->has_hill())
		{
			locPtr->set_fire_src(-100);
		}
		else if( locPtr->is_wall() )
		{
			locPtr->set_fire_src(-50);
		}
		else if( locPtr->is_firm() || locPtr->is_plant() || locPtr->is_town() )
		{
			locPtr->set_fire_src(100);
		}
		else
		{
			switch(terrain_res[locPtr->terrain_id]->average_type)
			{
			case TERRAIN_OCEAN:
				locPtr->set_fire_src(-100);
				break;
			case TERRAIN_DARK_GRASS:
				locPtr->set_fire_src(100);
				break;
			case TERRAIN_LIGHT_GRASS:
				locPtr->set_fire_src(50);
				break;
			case TERRAIN_DARK_DIRT:
				locPtr->set_fire_src(-50);
				break;
			default:
				err_now("Undefined terrain type");
			}
		}
		// --------- put off fire on the map ----------//
		locPtr->set_fire_str(-100);
	}
}
// ----------- end of function World::init_fire ---------- //


// ----------- begin of function World::spread_fire ---------- //
void World::spread_fire(Weather &w)
{
	
	char fireValue;
	int x,y;
	Location *locPtr;

	// -------- normalize wind_speed between -WIND_SPREADFIRE*SPREAD_RATE to +WIND_SPREADFIRE*SPREAD_RATE -------
	int windCos = int(w.wind_speed() * cos(w.wind_direct_rad()) / 100.0 * SPREAD_RATE * WIND_SPREADFIRE);
	int windSin = int(w.wind_speed() * sin(w.wind_direct_rad()) / 100.0 * SPREAD_RATE * WIND_SPREADFIRE);
	char rainSnowReduction = 0;
	
	rainSnowReduction = (w.rain_scale() > 0 || w.snow_scale() > 0) ? 
		RAIN_SNOW_REDUCTION + (w.rain_scale() + w.snow_scale()) / 4: 0;

	float flameDamage = (float)config.fire_damage/ATTACK_SLOW_DOWN;

	// -------------update fire_level-----------
	for( y = scan_fire_y; y < max_y_loc; y += SCAN_FIRE_DIST)
	{
		locPtr = get_loc(scan_fire_x,y);
		for( x = scan_fire_x; x < max_x_loc; x += SCAN_FIRE_DIST, locPtr+=SCAN_FIRE_DIST)
		{
			char oldFireValue = fireValue = locPtr->fire_str();
			char flammability = locPtr->fire_src();


			// ------- reduce fire_level on raining or snow
			fireValue -= rainSnowReduction;
			if(fireValue < -100)
				fireValue = -100;

			if( fireValue > 0)
			{
				Unit *targetUnit;

				// ------- burn wall -------- //
				if( locPtr->is_wall() )
				{
					if( !locPtr->attack_wall(int(4.0*flameDamage)))
						correct_wall(x, y, 2);
				}
				// ------- burn units ---------//
				else if( locPtr->has_unit(UNIT_LAND))
				{
					targetUnit = unit_array[locPtr->unit_recno(UNIT_LAND)];
					targetUnit->hit_points -= (float)2.0*flameDamage;
					if( targetUnit->hit_points <= 0 )
						targetUnit->hit_points = (float) 0;
				}
				else if( locPtr->has_unit(UNIT_SEA))
				{
					targetUnit = unit_array[locPtr->unit_recno(UNIT_SEA)];
					targetUnit->hit_points -= (float)2.0*flameDamage;
					if( targetUnit->hit_points <= 0 )
						targetUnit->hit_points = (float) 0;
				}
				else if( locPtr->is_firm() && firm_res[firm_array[locPtr->firm_recno()]->firm_id]->buildable)
				{
					Firm *targetFirm = firm_array[locPtr->firm_recno()];
					//### begin alex 6/8 ###//
					#ifdef DEBUG
					if(debug_sim_game_type!=2)
					#endif
					//#### end alex 6/8 ####//
					targetFirm->hit_points -= flameDamage;
					if( targetFirm->hit_points <= 0)
					{
						targetFirm->hit_points = (float) 0;
						// ###### begin Gilbert 29/5 ########//
						se_res.sound(targetFirm->center_x, targetFirm->center_y, 1,
							'F', targetFirm->firm_id, "DIE" );
						// ###### end Gilbert 29/5 ########//
						firm_array.del_firm(locPtr->firm_recno());
					}
				}

				if(SPREAD_RATE > 0)
				{

					Location *sidePtr;
					// spread of north square
					if( y>0 && (sidePtr = get_loc(x,y-1))->fire_src() >0
						&& sidePtr->fire_str() <= 0)
					{
						sidePtr->add_fire_str(bound_zero(char(SPREAD_RATE+windCos)));
					}

					// spread of south square
					if( y<max_y_loc-1 && (sidePtr = get_loc(x,y+1))->fire_src() >0
						&& sidePtr->fire_str() <= 0)
					{
						sidePtr->add_fire_str(bound_zero(char(SPREAD_RATE-windCos)));
					}

					// spread of east square
					if( x<max_x_loc-1 && (sidePtr = get_loc(x+1,y))->fire_src() >0
						&& sidePtr->fire_str() <= 0)
					{
						sidePtr->add_fire_str(bound_zero(char(SPREAD_RATE+windSin)));
					}

					// spread of west square
					if( x>0 && (sidePtr = get_loc(x-1,y))->fire_src() >0
						&& sidePtr->fire_str() <= 0)
					{
						sidePtr->add_fire_str(bound_zero(char(SPREAD_RATE-windSin)));
					}
				}

				if( flammability > 0)
				{
					// increase fire_level on its own
					if(++fireValue > 100)
						fireValue = 100;

					flammability -= FIRE_FADE_RATE;
					// if a plant on it then remove the plant, if flammability <= 0
					if( locPtr->is_plant() && flammability <= 0)
					{
						locPtr->remove_plant();
						plant_count--;
					}

				}
				else
				{
					// fireValue > 0, flammability < 0
					// putting of fire
					if( flammability >= -30)
					{
						fireValue-=2;
						flammability -= FIRE_FADE_RATE;
						if( flammability < -30)
							flammability = -30;
					}
					else if (flammability >= -50)
					{
						fireValue-=2;
						flammability -= FIRE_FADE_RATE;
						if( flammability < -50)
							flammability = -50;
					}
					else
					{
						fireValue = -100;
						flammability -= FIRE_FADE_RATE;
						if( flammability < -100)
							flammability = -100;
					}

					// if a plant on it then remove the plant, if flammability <= 0
					if( locPtr->is_plant() && flammability <= 0)
					{
						locPtr->remove_plant();
						plant_count--;
					}
				}
			}
			else
			{
				// fireValue < 0
				// ---------- fire_level drop slightly ----------
				if( fireValue > -100)
					fireValue--;
				
				// ---------- restore flammability ------------
				if( flammability >= -30 && flammability < 50 &&
					m.random(100) < RESTORE_RATE)
					flammability++;
			}

			// ---------- update new fire level -----------
			//-------- when fire is put off
			// so the fire will not light again very soon
			if(fireValue <= 0 && oldFireValue > 0)
			{
				fireValue -= 50;
			}

			locPtr->set_fire_str(fireValue);
			locPtr->set_fire_src(flammability);
		}
	}
}
//----------- end of function World::spread_fire ---------- //


//----------- begin of function World::setup_fire ---------- //
//
// set a fire at location x, y
// fireStrength is between 1 - 100, (default: 30)
//
void World::setup_fire(short x, short y, char fireStrength)
{
	err_when( x < 0 || y < 0 || x >= max_x_loc || y >= max_y_loc);
	err_when(fireStrength < 0 || fireStrength > 100);

	Location *locPtr = get_loc(x, y);
	if( locPtr->fire_str() < fireStrength )
	{
		locPtr->set_fire_str(fireStrength);
	}
}
// ----------- end of function World::setup_fire ---------- //

