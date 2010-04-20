/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : OTOWNA.CPP
//Description : Object Town Array

#include <stdlib.h>
#include <OBOX.h>
#include <OWORLD.h>
#include <OINFO.h>
#include <OCONFIG.h>
#include <OSYS.h>
#include <OTOWN.h>
#include <OF_MARK.h>
#include <OF_MONS.h>
#include <ONATION.h>
#include <OGAME.h>

#ifdef DEBUG
#include <OFONT.h>

//### begin alex 20/9 ###//
static unsigned long	last_town_ai_profile_time = 0L;
static unsigned long	town_ai_profile_time = 0L;
static unsigned long	last_town_profile_time = 0L;
static unsigned long	town_profile_time = 0L;
//#### end alex 20/9 ####//
#endif

//--------- Begin of function TownArray::TownArray ----------//

TownArray::TownArray() : DynArrayB(sizeof(Town*), 10, DEFAULT_REUSE_INTERVAL_DAYS)
{
}
//--------- End of function TownArray::TownArary ----------//


//------- Begin of function TownArray::~TownArray ----------//
//
TownArray::~TownArray()
{
	deinit();
}
//--------- End of function TownArray::~TownArray ----------//


//--------- Begin of function TownArray::init ----------//
//
void TownArray::init()
{
	memset( race_wander_pop_array, 0, sizeof(race_wander_pop_array) );
}
//---------- End of function TownArray::init ----------//


//--------- Begin of function TownArray::deinit ----------//
//
void TownArray::deinit()
{
	//----- delete Town objects ------//

	if( size() > 0 )
	{
		Town* townPtr;

		for( int i=1 ; i<=size() ; i++ )
		{
			townPtr = (Town*) get_ptr(i);

			if( townPtr )
				delete townPtr;
		}

		zap();
	}
}
//---------- End of function TownArray::deinit ----------//


//----- Begin of function TownArray::create_town -------//
//
// Create a blank town object and link it into town_array.
//
Town* TownArray::create_town()
{
	Town* townPtr;

	townPtr = new Town;

	linkin(&townPtr);

	return townPtr;
}
//------- End of function TownArray::create_town -------//


//----- Begin of function TownArray::add_town -------//
//
// <int> nationRecno - the nation recno
// <int> raceId 	   - the race of the majority of the town poulation
// <int> xLoc, yLoc  - location of the town
//
int TownArray::add_town(int nationRecno, int raceId, int xLoc, int yLoc)
{
	Town* townPtr;

	townPtr = new Town;

	linkin(&townPtr);

	townPtr->town_recno = recno();
	townPtr->init(nationRecno, raceId, xLoc, yLoc);

	nation_array.update_statistic();		// update largest_town_recno

	return recno();
}
//------- End of function TownArray::add_town -------//


//----- Begin of function TownArray::del_town -------//
//
void TownArray::del_town(int townRecno)
{
	Town* townPtr = operator[](townRecno);

	townPtr->deinit();
	delete townPtr;

	linkout(townRecno);

	nation_array.update_statistic();
}
//------- End of function TownArray::del_town -------//


//--------- Begin of function TownArray::process ---------//
//
// Process all town in town_array for action and movement for next frame
//
void TownArray::process()
{
	//----- call Town::next_day --------//

	int  i;
	Town *townPtr;

	for( i=size() ; i>0 ; i-- )
	{
		townPtr = (Town*) get_ptr(i);

		if( !townPtr )
			continue;

		//------- if all the population are gone --------//

		if( townPtr->population==0 )
		{
			del_town(i);
			continue;
		}

		//-------------------------------//

		err_when(town_array.is_deleted(i));
		err_when( townPtr->town_recno==0 );

		if( i%FRAMES_PER_DAY == int(sys.frame_count%FRAMES_PER_DAY) )	// only process each firm once per day
		{
			err_when( townPtr->town_recno==0 );

			if( townPtr->nation_recno==0 )
			{
				townPtr->think_independent_town();
			}
			else
			{
				#ifdef DEBUG
				if(!config.disable_ai_flag && townPtr->ai_town)
				#else
				if( townPtr->ai_town )
				#endif
				{
					#ifdef DEBUG
					unsigned long profileStartTime = m.get_time();
					#endif

					townPtr->process_ai();

					#ifdef DEBUG
					town_profile_time += m.get_time() - profileStartTime;
					#endif
				}
			}

			if( town_array.is_deleted(i) )
				continue;

			err_when( townPtr->town_recno==0 );

			//### begin alex 20/9 ###//
			#ifdef DEBUG
			unsigned long profileStartTime = m.get_time();
			#endif

			townPtr->next_day();

			#ifdef DEBUG
			town_profile_time += m.get_time() - profileStartTime;
			#endif
			//#### end alex 20/9 ####//
		}
	}

	//------ distribute demand -------//

	if( sys.day_frame_count==0 && info.game_date%15==0 )			// distribute demand every 15 days
		distribute_demand();

	//------ create new independent town -----//

	if( info.game_date%30==0 && sys.frame_count%FRAMES_PER_DAY==0 )
		think_new_independent_town();
}
//----------- End of function TownArray::process ---------//


//----- Begin of function TownArray::think_new_independent_town -----//
//
// Think about creating new independent towns.
//
void TownArray::think_new_independent_town()
{
	if( m.random(3) != 0 )		// 1/3 chance
		return;

	//---- count the number of independent towns ----//

	Town* townPtr;
	int   independentTownCount=0, allTotalPop=0;

	int i;
	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		allTotalPop += townPtr->population;

		if( townPtr->nation_recno == 0 )
			independentTownCount++;
	}

	if( independentTownCount >= 10 )		// only when the no. of independent town is less than 10
		return;

	//--- if the total population of all nations combined > 1000, then no new independent town will emerge ---//

	if( allTotalPop > 1000 )
		return;

	//--- add 1 to 2 wanderer per month per race ---//

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		race_wander_pop_array[i] += 2+m.random(5);
	}

	//----- check if there are enough wanderers to set up a new town ---//

	int raceId = m.random(MAX_RACE)+1;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( ++raceId > MAX_RACE )
			raceId = 1;

		if( race_wander_pop_array[raceId-1] >= 10 )	// one of the race must have at least 10 people
			break;
	}

	if( i==MAX_RACE )
		return;

	//------- locate for a space to build the town ------//

	int xLoc, yLoc;

	if( !think_town_loc(MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4, xLoc, yLoc) )
		return;

	//--------------- create town ---------------//

	int townRecno  = town_array.add_town(0, raceId, xLoc, yLoc);
	int maxTownPop = 20 + m.random(10);
	int addPop, townResistance;
	int loopCount=0;

	townPtr = town_array[townRecno];

	while(1)
	{
		err_when( loopCount++ > 100 );

		addPop = race_wander_pop_array[raceId-1];
		addPop = MIN(maxTownPop-townPtr->population, addPop);

		townResistance = independent_town_resistance();

		townPtr->init_pop( raceId, addPop, townResistance, 0, 1 );		// 0-the add pop do not have jobs, 1-first init

		race_wander_pop_array[raceId-1] -= addPop;

		err_when( race_wander_pop_array[raceId-1] < 0 );

		if( townPtr->population >= maxTownPop )
			break;

		//---- next race to be added to the independent town ----//

		raceId = m.random(MAX_RACE)+1;

		for( i=0 ; i<MAX_RACE ; i++ )
		{
			if( ++raceId > MAX_RACE )
				raceId = 1;

			if( race_wander_pop_array[raceId-1] >= 5 )
				break;
		}

		if( i==MAX_RACE )		// no suitable race
			break;
	}

	//---------- set town layout -----------//

	townPtr->auto_set_layout();

//	if( sys.debug_session )
//		box.msg( "A new independent town has emerged." );
}
//------ End of function TownArray::think_new_independent_town -----//


//----- Begin of function TownArray::independent_town_resistance ------//
//
// Return a random resistance value for new independent town.
//
int TownArray::independent_town_resistance()
{
	switch(config.independent_town_resistance)
	{
		case OPTION_LOW:
			return 40 + m.random(20);
			break;

		case OPTION_MODERATE:
			return 50 + m.random(30);
			break;

		case OPTION_HIGH:
			return 60 + m.random(40);
			break;

		default:
			err_here();
			return 60 + m.random(40);
	}
}
//----- End of function TownArray::independent_town_resistance ------//


//-------- Begin of function TownArray::think_town_loc --------//
//
// Locate for an area of free space
//
// <int>  maxTries       = maximum no. of tries
// <int&> xLoc           = for returning result
// <int&> yLoc           = for returning result
//
// return : <int> 1 - free space found
//                0 - free space found
//
int TownArray::think_town_loc(int maxTries, int& xLoc, int& yLoc)
{
	#define MIN_INTER_TOWN_DISTANCE  16
	#define BUILD_TOWN_LOC_WIDTH     16
	#define BUILD_TOWN_LOC_HEIGHT    16

	int       i, x, y, canBuildFlag, townRecno, firmRecno;
	Location* locPtr;
	Town*     townPtr;
	Firm* 	 firmPtr;

   for( i=0 ; i<maxTries ; i++ )
	{
		xLoc = m.random(MAX_WORLD_X_LOC-BUILD_TOWN_LOC_WIDTH);
		yLoc = 2+m.random(MAX_WORLD_Y_LOC-BUILD_TOWN_LOC_HEIGHT-2);		// do not build on the upper most location as the flag will go beyond the view area

		canBuildFlag=1;

		//---------- check if the area is all free ----------//

		for( y=yLoc ; y<yLoc+BUILD_TOWN_LOC_HEIGHT ; y++ )
		{
			locPtr = world.get_loc(xLoc, y);

			for( x=xLoc ; x<xLoc+BUILD_TOWN_LOC_WIDTH ; x++, locPtr++ )
			{
				if( !locPtr->can_build_town() )
				{
					canBuildFlag=0;
					break;
				}
			}
		}

		if( !canBuildFlag )
			continue;

		//-------- check if it's too close to other towns --------//

		for( townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
		{
			if( town_array.is_deleted(townRecno) )
				continue;

			townPtr = town_array[townRecno];

			if( m.points_distance(xLoc+1, yLoc+1, townPtr->center_x,		// xLoc+1 and yLoc+1 to take the center location of the town
				 townPtr->center_y) < MIN_INTER_TOWN_DISTANCE )
			{
				break;
			}
		}

		if( townRecno > 0 )	// if it's too close to other towns
			continue;

		//-------- check if it's too close to monster firms --------//

		for( firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
		{
			if( firm_array.is_deleted(firmRecno) )
				continue;

			firmPtr = firm_array[firmRecno];

			if( m.points_distance(xLoc+1, yLoc+1, firmPtr->center_x,
				 firmPtr->center_y) < MONSTER_ATTACK_NEIGHBOR_RANGE )
			{
				break;
			}
		}

		if( firmRecno > 0 )     // if it's too close to monster firms
			continue;

		//----------------------------------------//

		return 1;
	}

	return 0;
}
//--------- End of function TownArray::think_town_loc ---------//


//--------- Begin of function TownArray::draw ---------//
//
// Draw town towns on the zoom map.
//
void TownArray::draw()
{
	int  i;
	Town *townPtr;

	for( i=size() ; i>0 ; i-- )
	{
		townPtr = (Town*) get_ptr(i);

		if( townPtr )
			townPtr->draw();
	}
}
//----------- End of function TownArray::draw ---------//


//--------- Begin of function TownArray::draw_dot ---------//
//
// Draw tiny dots on map window representing the location of the town town.
//
void TownArray::draw_dot()
{
	char*	  		vgaBufPtr = vga_back.buf_ptr();
	char*			writePtr;
	int	  		i, x, y;
	Town*    	townPtr;
	TownLayout* townLayout;
	char			nationColor;
	char*   	   nationColorArray = nation_array.nation_color_array;
	int			vgaBufPitch = vga_back.buf_pitch();

	// ##### begin Gilbert 16/8 #######//
	const unsigned int excitedColorCount = 4;
	char excitedColorArray[MAX_NATION+1][excitedColorCount];
	for( i = 0; i <= MAX_NATION; ++i )
	{
		if( i == 0 || !nation_array.is_deleted(i) )
		{
			char *remapTable = game.get_color_remap_table(i, 0);
			excitedColorArray[i][0] = remapTable[0xe0];
			excitedColorArray[i][1] = remapTable[0xe1];
			excitedColorArray[i][2] = remapTable[0xe2];
			excitedColorArray[i][3] = remapTable[0xe3];
		}
		else
		{
			excitedColorArray[i][0] = 
			excitedColorArray[i][1] = 
			excitedColorArray[i][2] = 
			excitedColorArray[i][3] = (char) V_WHITE;
		}
	}
	// ##### end Gilbert 16/8 #######//

	for(i=1; i <=size() ; i++)
	{
		townPtr = (Town*) get_ptr(i);

		if( !townPtr)
			continue;

		nationColor = info.game_date - townPtr->last_being_attacked_date > 2 ?
			nationColorArray[townPtr->nation_recno] :
			excitedColorArray[townPtr->nation_recno][sys.frame_count % excitedColorCount];

		townLayout = town_res.get_layout(townPtr->layout_id);

		writePtr = vgaBufPtr + (MAP_Y1+townPtr->loc_y1)*vgaBufPitch + (MAP_X1+townPtr->loc_x1);

		char shadowColor = (char) VGA_GRAY;
		for( y=STD_TOWN_LOC_HEIGHT ; y>0 ; y--, writePtr+=vgaBufPitch-STD_TOWN_LOC_WIDTH )
		{
			for( x=STD_TOWN_LOC_WIDTH ; x>0 ; x--, writePtr++ )
			{
				if( *writePtr != UNEXPLORED_COLOR )
					*writePtr = nationColor;
			}

			if( *(writePtr+vgaBufPitch) != UNEXPLORED_COLOR)
				*(writePtr+vgaBufPitch) = shadowColor;
		}
		for( x = STD_TOWN_LOC_WIDTH ; x>0; x--)
		{
			if( *(++writePtr) != UNEXPLORED_COLOR )
				*writePtr = shadowColor;
		}
	}
}
//----------- End of function TownArray::draw_dot -----------//


//--------- Begin of function TownArray::draw_profile ---------//
void TownArray::draw_profile()
{
#ifdef DEBUG
	static unsigned long lastDrawTime = m.get_time();

	//### begin alex 20/9 ###//
	if(m.get_time() >= lastDrawTime + 1000)
	{
		last_town_ai_profile_time = town_ai_profile_time;
		town_ai_profile_time = 0L;
		last_town_profile_time = town_profile_time;
		town_profile_time = 0L;
		lastDrawTime = m.get_time();
	}

	String str;
	str  = "Town  : ";
	font_news.disp( ZOOM_X1+10, ZOOM_Y1+90, str, MAP_X2);

	str = "";
	str += last_town_ai_profile_time;
	font_news.disp( ZOOM_X1+60, ZOOM_Y1+90, str, MAP_X2);

	str = "";
	str += last_town_profile_time;
	font_news.disp( ZOOM_X1+100, ZOOM_Y1+90, str, MAP_X2);
	//#### end alex 20/9 ####//
#endif
}
//----------- End of function TownArray::draw_profile -----------//


//--------- Begin of function TownArray::find_nearest_town --------//
//
// Find the town that is nearest to the firm.
//
// <int>  xLoc, yLoc   = the x and y location of the town
// [int]  nationRecno  = nation recno of the town town
//								 (default: 0, any nation)
//
// return : <int> town town recno of where the firm is nearest to
//
int TownArray::find_nearest_town(int xLoc, int yLoc, int nationRecno)
{
	int   i, curDistance, bestTownRecno=0, minDistance=0x7FFFFFFF;
	Town* townPtr;

	for( i=size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		curDistance = m.points_distance( xLoc, yLoc, townPtr->center_x, townPtr->center_y );

		if( nationRecno && townPtr->nation_recno != nationRecno )
			continue;

		//--- when the firm is outside the town, find the town nearest to the firm ---//

		if( curDistance < minDistance )
		{
			minDistance = curDistance;
			bestTownRecno = i;
		}
	}

	return bestTownRecno;
}
//----------- End of function TownArray::find_nearest_town ---------//


//--------- Begin of function TownArray::settle --------//
//
// A unit settles on a given location and form a town town.
//
// return: <int> 1 - settled and a town town is formed successfully.
//					  0 - settling failed. too close to a town of another nation.
//
int TownArray::settle(int unitRecno, int xLoc, int yLoc)
{
	if(!world.can_build_town(xLoc, yLoc, unitRecno))
		return 0;

	//--------- get the nearest town town ------------//

	Unit* unitPtr = unit_array[unitRecno];

	char nationRecno = unitPtr->nation_recno;

	//----- it's far enough to form another town --------//

	int townRecno = town_array.add_town( nationRecno, unitPtr->race_id, xLoc, yLoc );

	//---------- init town population ----------//

	Town* townPtr = town_array[townRecno];

	//----------------------------------------------------//
	// if the settle unit is standing in the town area
	// cargo_recno of that location is unchange in town_array.add_town
	// so update the location now
	//----------------------------------------------------//

	short uXLoc = unitPtr->next_x_loc();
	short uYLoc = unitPtr->next_y_loc();
	Location *locPtr = world.get_loc(uXLoc, uYLoc);

	townPtr->assign_unit(unitRecno);

	if( uXLoc>=townPtr->loc_x1 && uXLoc<=townPtr->loc_x2 && uYLoc>=townPtr->loc_y1 && uYLoc<=townPtr->loc_y2 )
		locPtr->set_town(townPtr->town_recno);

#ifdef DEBUG
	// make sure cargo recno is set to town's
	for( uYLoc = townPtr->loc_y1; uYLoc <= townPtr->loc_y2; ++uYLoc)
	{
		for( uXLoc = townPtr->loc_x1; uXLoc <= townPtr->loc_x2; ++uXLoc)
		{
			err_when( world.get_loc(uXLoc, uYLoc)->town_recno() != townPtr->town_recno );
		}
	}
#endif

	townPtr->update_target_loyalty();

	//--------- hide the unit from the map ----------//

	return 1;
}
//----------- End of function TownArray::settle ---------//


//--------- Begin of function TownArray::distribute_demand --------//
//
void TownArray::distribute_demand()
{
	//--- reset market place demand ----//

	int i, j;
	FirmMarket* firmMarket;

	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i)  )
			continue;

		firmMarket = (FirmMarket*) firm_array[i];

		if( firmMarket->firm_id == FIRM_MARKET )
		{
			for( j=0 ; j<MAX_MARKET_GOODS ; j++ )
			{
				firmMarket->market_goods_array[j].month_demand = (float) 0;
			}
		}
	}

	//-------- distribute demand -------//

	for( i=size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		town_array[i]->distribute_demand();
	}
}
//----------- End of function TownArray::distribute_demand ---------//


//------- Begin of function TownArray::stop_attack_nation -----//
void TownArray::stop_attack_nation(short nationRecno)
{
	Town* townPtr;
	for(int i=size(); i>0; --i)
	{
		townPtr = (Town*) get_ptr(i);

		if(townPtr)
			townPtr->reset_hostile_nation(nationRecno);
	}
}
//----------- End of function TownArray::stop_attack_nation ---------//


//------- Begin of function TownArray::is_deleted -----//

int TownArray::is_deleted(int recNo)
{
	Town *townPtr = (Town*) get_ptr(recNo);

	if( !townPtr )
		return 1;

	if( townPtr->population==0 )
		return 1;

	return 0;
}
//--------- End of function TownArray::is_deleted ----//


#ifdef DEBUG

//------- Begin of function TownArray::operator[] -----//

Town* TownArray::operator[](int recNo)
{
	Town* townPtr = (Town*) get_ptr(recNo);

	if( !townPtr )
		err.run( "TownArray[] is deleted" );

	return townPtr;
}

//--------- End of function TownArray::operator[] ----//

#endif

