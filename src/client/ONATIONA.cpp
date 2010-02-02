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

//Filename    : ONATIONA.CPP
//Description : Object Nation Array

#include <COLOR.h>
#include <OPOWER.h>
#include <OGAME.h>
#include <OF_CAMP.h>
#include <OUNIT.h>
#include <OINFO.h>
#include <OSYS.h>
#include <ORACERES.h>
#include <OSPY.h>
#include <ONATIONA.h>
#include <OREMOTE.h>
#include <OLOG.h>

//### begin alex 22/9 ###//
#ifdef DEBUG
#include <OFONT.h>

static unsigned long	last_nation_ai_profile_time = 0L;
static unsigned long	nation_ai_profile_time = 0L;
static unsigned long	last_nation_profile_time = 0L;
static unsigned long	nation_profile_time = 0L;
#endif
//#### end alex 22/9 ####//


//--------- Begin of function NationArray::NationArray ---------//
//
NationArray::NationArray() : DynArrayB( sizeof(Nation*), MAX_NATION )
{
}
//----------- End of function NationArray::NationArray ---------//


//--------- Begin of function NationArray::~NationArray ---------//
//
NationArray::~NationArray()
{
	deinit();
}
//----------- End of function NationArray::~NationArray ---------//


//--------- Begin of function NationArray::init ---------//
//
void NationArray::init()
{
	nation_count    = 0;
	ai_nation_count = 0;

	last_del_nation_date = 0;
	nation_peace_days    = 0;
	last_alliance_id     = 0;

	nation_color_array[0] 		 = (char) V_WHITE;						// if nation_recno==0, the color is white
	nation_power_color_array[0] = (char) VGA_GRAY+10;						// if nation_recno==0, the color is white
	nation_power_color_array[MAX_NATION+1] = (char) VGA_GRAY+10;	// if Location::power_nation_recno==MAX_NATION+1, this means there are more than one nation have influence over this location.
}
//----------- End of function NationArray::init ---------//


//--------- Begin of function NationArray::deinit ---------//
//
// Called by ~NationArray when game terminate
// and NationArray::read_file() when loading game
//
void NationArray::deinit()
{
	int i;     // can't call the destructor of Firm, must call deinit()

   for( i=size() ; i>0 ; i-- )
	{
      if( !is_deleted(i) )
         del_nation(i);
   }

   zap();       // clean up the array
}
//----------- End of function NationArray::deinit ---------//


//--------- Begin of function NationArray::new_nation ---------//
//
// <int>  nationType  		   = NATION_OWN, NATION_AI, or NATION_REMOTE
// <int>  raceId      		   = id. of the race
// <int>  colorSchemeId 		= the color scheme id. of the nation
// [unsigned long] dpPlayerId = DirectPlayer player id. (only for multiplayer game)
//
// return : <int> nationRecno = the recno. of the newly added nation
//
int NationArray::new_nation(int nationType, int raceId, int colorSchemeId, unsigned long dpPlayerId)
{
	err_when( info.game_date < last_del_nation_date + NEW_NATION_INTERVAL_DAYS );

	//--------------------------------------------------------//

	int     nationRecno = create_nation();
	Nation* nationPtr   = nation_array[nationRecno];

	if( nationType == NATION_OWN )
	{
		player_ptr   = nationPtr;      // for ~nation_array to quick access
		player_recno = nationRecno;

		info.default_viewing_nation_recno = nationRecno;
		info.viewing_nation_recno = nationRecno;
	}

	//--- we must call init() after setting ai_type & nation_res_id ----//

	nationPtr->init(nationType, raceId, colorSchemeId, dpPlayerId);

	//--- store the colors of all nations into a single array for later fast access ---//

	nation_color_array[nationPtr->nation_recno] = nationPtr->nation_color;
	nation_power_color_array[nationPtr->nation_recno] = nationPtr->nation_color; 	// use a lighter color for the nation power area

	//-------------------------------------------//

	nation_count++;

	if( nationType == NATION_AI )
		ai_nation_count++;

	last_new_nation_date = info.game_date;

	//---------- update statistic ----------//

	update_statistic();

	return nationRecno;
}
//----------- End of function NationArray::new_nation ---------//


// ######## begin Gilbert 18/8 #########//
//--------- Begin of function NationArray::new_nation ---------//
//
// used in creating human players in multi-player game
// return : <int> nationRecno = the recno. of the newly added nation
//
int NationArray::new_nation(NewNationPara& nationPara)
{
	err_when( info.game_date < last_del_nation_date + NEW_NATION_INTERVAL_DAYS );

	//--------------------------------------------------------//

	
	int     nationRecno = create_nation();
	Nation* nationPtr   = nation_array[nationRecno];

	err_when( nationRecno != nationPara.nation_recno );
	err_when( !remote.is_enable() );

	char nationType = 
		nationPara.dp_player_id == remote.self_player_id() ? NATION_OWN : NATION_REMOTE;

	if( nationType == NATION_OWN )
	{
		player_ptr   = nationPtr;      // for ~nation_array to quick access
		player_recno = nationRecno;

		info.default_viewing_nation_recno = nationRecno;
		info.viewing_nation_recno = nationRecno;
	}

	//--- we must call init() after setting ai_type & nation_res_id ----//

	nationPtr->init(nationType, nationPara.race_id, nationPara.color_scheme, nationPara.dp_player_id);

	//--- store the colors of all nations into a single array for later fast access ---//

	nation_color_array[nationPtr->nation_recno] = nationPtr->nation_color;
	nation_power_color_array[nationPtr->nation_recno] = nationPtr->nation_color; 	// use a lighter color for the nation power area

	//-------------------------------------------//

	nation_count++;

	if( nationType == NATION_AI )
		ai_nation_count++;

	//---------- update statistic ----------//

	update_statistic();

	return nationRecno;
}
//----------- End of function NationArray::new_nation ---------//
// ######## end Gilbert 18/8 #########//


//--------- Begin of function NationArray::create_nation ---------//
//
// <int> nationType = NATION_OWN, NATION_AI
//
// return : <int> nationRecno = the recno. of the newly added nation
//
int NationArray::create_nation()
{
	Nation *nationPtr = new Nation;   // NATION_OWN, NATION_REMOTE also use
												// may be switched to player nation and verse vice
	linkin(&nationPtr);

	err_when( size() > MAX_NATION );

	nationPtr->nation_recno = recno();

	return recno();
}
//----------- End of function NationArray::create_nation ---------//


//--------- Begin of function NationArray::nation_class_size ---------//
//
// Return the size of the nation class
//
// <int>   nationType  = nation type, e.g. NATION_OWN, NATION_AI
//
int NationArray::nation_class_size()
{
	return sizeof(Nation);		// NATION_OWN, NATION_REMOTE also use
										// Nation object because a AI nation
										// may be switched to player nation and verse vice
}
//----------- End of function NationArray::nation_class_size ---------//


//--------- Begin of function NationArray::del_nation ---------//
//
// <int> recNo = the no. of the record to be deleted
//               (default : current record no.)
//
void NationArray::del_nation(int recNo)
{
	//-----------------------------------------//

	Nation* nationPtr = nation_array[recNo];       // operator[] will check for deleted nation error, can't use operator() because it use firm_array()->nation_recno

	nation_count--;

	if( nationPtr->nation_type == NATION_AI )
		ai_nation_count--;

	last_del_nation_date = info.game_date;

	//------ delete and linkout the nation ------//

	nationPtr->deinit();

	delete nationPtr;

	go(recNo);

	*( (Nation**) get() ) = NULL;   // Nullify the pointer
	linkout();

	//---- if the nation to be deleted is the player's nation ---//

	if( recNo==player_recno )
	{
		player_ptr   = NULL;
		player_recno = 0;

		if( !sys.signal_exit_flag )
			sys.set_view_mode(MODE_NORMAL);		// set the view mode to normal mode to prevent possible problems
	}

	//---------- update statistic ----------//

	update_statistic();		// as max_overall_nation_recno and others may be pointing to the deleted nation
}
//----------- End of function NationArray::del_nation ---------//


//--------- Begin of function NationArray::process ---------//
//
void NationArray::process()
{
	int  i;
	Nation *nationPtr;

	for( i=size() ; i>0 ; i-- )
	{
		nationPtr = (Nation*) get_ptr(i);

		if( !nationPtr )
			continue;

		//--------- process nation --------//

		if( i%FRAMES_PER_DAY == int(sys.frame_count%FRAMES_PER_DAY) )	// only process each firm once per day
		{
			//### begin alex 22/9 ###//
			#ifdef DEBUG
			unsigned long profileStartTime = m.get_time();
			#endif
			//#### end alex 22/9 ####//

			// ###### begin Gilbert 6/9 ######//
			LOG_MSG(i);
			LOG_MSG("begin next_day");
			nationPtr->next_day();
			LOG_MSG("end next_day");
			LOG_MSG(m.get_random_seed());
			// ###### end Gilbert 6/9 ######//

			//### begin alex 22/9 ###//
			#ifdef DEBUG
			nation_profile_time += m.get_time() - profileStartTime;
			#endif
			//#### end alex 22/9 ####//

			if( nation_array.is_deleted(i) )
				continue;

			// ###### begin Gilbert 6/9 ######//
			if( nationPtr->nation_type == NATION_AI )
			{
				#ifdef DEBUG
				unsigned long profileAiStartTime = m.get_time();
				#endif

				LOG_MSG( "begin process_ai");
				nationPtr->process_ai();
				LOG_MSG( "end process_ai");
				LOG_MSG( m.get_random_seed());

				#ifdef DEBUG
				nation_profile_time += m.get_time() - profileStartTime;
				#endif
			}
			// ###### end Gilbert 6/9 ######//
		}
	}

	if( sys.frame_count%FRAMES_PER_DAY==0 )
		nation_peace_days++;
}
//----------- End of function NationArray::process ---------//


//--------- Begin of function NationArray::next_month ---------//
//
void NationArray::next_month()
{
	int  i;
	Nation *nationPtr;

	// ###### begin Gilbert 6/9 ########//
	LOG_MSG("begin NationArray::next_month");
	for( i=size() ; i>0 ; i-- )
	{
		nationPtr = (Nation*) get_ptr(i);

		if( nationPtr )    // nationPtr == NULL, if it's deleted
		{
			LOG_MSG(i);
			nationPtr->next_month();
			LOG_MSG(m.get_random_seed() );
		}
	}
	LOG_MSG("end NationArray::next_month");
	// ###### end Gilbert 6/9 ########//

	//------- update statistic -----------//

	update_statistic();
}
//---------- End of function NationArray::next_month ----------//


//--------- Begin of function NationArray::next_year ---------//
//
void NationArray::next_year()
{
	int  i;
	Nation *nationPtr;

	// ###### begin Gilbert 6/9 ########//
	LOG_MSG("begin NationArray::next_year");
	for( i=size() ; i>0 ; i-- )
	{
		nationPtr = (Nation*) get_ptr(i);

		if( nationPtr )            // nationPtr == NULL, if it's deleted
		{
			LOG_MSG(i);
			nationPtr->next_year();
			LOG_MSG(m.get_random_seed() );
		}
	}
	LOG_MSG("end NationArray::next_year");
	// ###### end Gilbert 6/9 ########//
}
//---------- End of function NationArray::next_year ----------//


//-------- Begin of function NationArray::random_unused_race --------//
//
// Randomly pick a race id. that hasn't been used by existing nations.
//
int NationArray::random_unused_race()
{
	//----- figure out which race has been used, which has not -----//

	char usedRaceArray[MAX_RACE];
	int  usedCount=0;

	memset( usedRaceArray, 0, sizeof(usedRaceArray) );

	int i;
	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		usedRaceArray[ nation_array[i]->race_id-1 ] = 1;
		usedCount++;
	}

	err_when( usedCount == MAX_RACE );

	//----- pick a race randomly from the unused list -----//

	int pickedInstance = m.random(MAX_RACE-usedCount)+1;
	int usedId=0;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( !usedRaceArray[i] )
		{
			usedId++;

			if( usedId == pickedInstance )
				return i+1;
		}
	}

	err_here();

	return 0;
}
//--------- End of function NationArray::random_unused_race ---------//


//-------- Begin of function NationArray::random_unused_color --------//
//
// Randomly pick a color that hasn't been used by existing nations.
//
int NationArray::random_unused_color()
{
	//----- figure out which race has been used, which has not -----//

	char usedColorArray[MAX_COLOR_SCHEME];
	int  usedCount=0;

	memset( usedColorArray, 0, sizeof(usedColorArray) );

	int i;
	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		usedColorArray[ nation_array[i]->color_scheme_id-1 ] = 1;
		usedCount++;
	}

	err_when( usedCount == MAX_COLOR_SCHEME );

	//----- pick a race randomly from the unused list -----//

	int pickedInstance = m.random(MAX_COLOR_SCHEME-usedCount)+1;
	int usedId=0;

	for( i=0 ; i<MAX_COLOR_SCHEME ; i++ )
	{
		if( !usedColorArray[i] )
		{
			usedId++;

			if( usedId == pickedInstance )
				return i+1;
		}
	}

	err_here();

	return 0;
}
//--------- End of function NationArray::random_unused_color ---------//


//-------- Begin of function NationArray::can_form_new_ai_nation --------//
//
// Return whether a new nation can be formed now.
//
int NationArray::can_form_new_ai_nation()
{
	return config.new_nation_emerge &&
			 nation_array.ai_nation_count < config.ai_nation_count &&
			 nation_array.nation_count < MAX_NATION &&
			 info.game_date > last_del_nation_date + NEW_NATION_INTERVAL_DAYS &&
			 info.game_date > last_new_nation_date + NEW_NATION_INTERVAL_DAYS;
}
//--------- End of function NationArray::can_form_new_ai_nation ---------//


//------- Begin of function NationArray::update_statistic ------//
//
void NationArray::update_statistic()
{
	//----- reset statistic vars first ------//

	int  i;
	Nation *nationPtr;

	for( i=size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		nationPtr = nation_array[i];

		nationPtr->total_population   	   = 0;
		nationPtr->total_jobless_population = 0;

		nationPtr->largest_town_recno = 0;
		nationPtr->largest_town_pop   = 0;

		nationPtr->total_spy_count    = 0;
		nationPtr->total_ship_combat_level = 0;
	}

	//------ calculate town statistic -------//

	independent_town_count = 0;
	memset( independent_town_count_race_array, 0, sizeof(independent_town_count_race_array) );	// the no. of independent towns each race has

	Town* townPtr;

	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->nation_recno )
		{
			Nation* nationPtr = nation_array[townPtr->nation_recno];

			nationPtr->total_population += townPtr->population;
			nationPtr->total_jobless_population += townPtr->jobless_population;

			if( townPtr->population > nationPtr->largest_town_pop )
			{
				nationPtr->largest_town_pop 	 = townPtr->population;
				nationPtr->largest_town_recno = i;
			}
		}
		else
		{
			independent_town_count++;

			//--- the no. of independent towns each race has ---//

			for( int j=0 ; j<MAX_RACE ; j++ )
			{
				if( townPtr->race_pop_array[j] >= 6 )     	// only count it if the pop of the race >= 6
					independent_town_count_race_array[j]++;
			}
		}
	}

	//------ calculate spy statistic -------//

	for( i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		nation_array[ spy_array[i]->true_nation_recno ]->total_spy_count++;
	}

	//--- update nation rating (this must be called after the above code, which update vars like total_population ---//

	for( i=size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		nation_array[i]->update_nation_rating();
	}

	//------ update the military rating of all nations -----//

	update_military_rating();

	//------ update the total_human_count of all nations -----//

//	#ifdef DEBUG
//	update_total_human_count();		//**BUGHERE this function to be deleted after checking
//	#endif

	//--------- update nation maximum ----------//

	max_nation_population = 0;
	all_nation_population = 0;

	max_nation_units		 = 0;
	max_nation_humans	    = 0;
	max_nation_generals	 = 0;
	max_nation_weapons	 = 0;
	max_nation_ships		 = 0;
	max_nation_spies		 = 0;

	max_nation_firms		 = 0;
	max_nation_tech_level = 0;

	max_population_rating  = -0x7FFF;
	max_military_rating	  = -0x7FFF;
	max_economic_rating	  = -0x7FFF;
	max_reputation			  = -0x7FFF;
	max_kill_monster_score = -0x7FFF;
	max_overall_rating	  = -0x7FFF;

	max_population_nation_recno   = 0;
	max_military_nation_recno	   = 0;
	max_economic_nation_recno	   = 0;
	max_reputation_nation_recno	= 0;
	max_kill_monster_nation_recno = 0;
	max_overall_nation_recno	   = 0;

	for( i=size() ; i>0 ; i-- )
	{
		nationPtr = (Nation*) get_ptr(i);

		if( !nationPtr )    // nationPtr == NULL, if it's deleted
			continue;

		all_nation_population += nationPtr->total_population;

		if( nationPtr->total_population > max_nation_population )
			max_nation_population = nationPtr->total_population;

		if( nationPtr->total_unit_count > max_nation_units )
			max_nation_units = nationPtr->total_unit_count;

		if( nationPtr->total_human_count > max_nation_humans )
			max_nation_humans = nationPtr->total_human_count;

		if( nationPtr->total_general_count > max_nation_generals )
			max_nation_generals = nationPtr->total_general_count;

		if( nationPtr->total_weapon_count > max_nation_weapons )
			max_nation_weapons = nationPtr->total_weapon_count;

		if( nationPtr->total_ship_count > max_nation_ships )
			max_nation_ships = nationPtr->total_ship_count;

		if( nationPtr->total_spy_count > max_nation_spies )
			max_nation_spies = nationPtr->total_spy_count;

		if( nationPtr->total_firm_count > max_nation_firms )
			max_nation_firms = nationPtr->total_firm_count;

		if( nationPtr->total_tech_level() > max_nation_tech_level )
			max_nation_tech_level = nationPtr->total_tech_level();

		//----- update maximum nation rating ------//

		if( nationPtr->population_rating > max_population_rating )
		{
			max_population_rating = nationPtr->population_rating;
			max_population_nation_recno = i;
		}

		if( nationPtr->military_rating > max_military_rating )
		{
			max_military_rating = nationPtr->military_rating;
			max_military_nation_recno = i;
		}

		if( nationPtr->economic_rating > max_economic_rating )
		{
			max_economic_rating = nationPtr->economic_rating;
			max_economic_nation_recno = i;
		}

		if( (int) nationPtr->reputation > max_reputation )
		{
			max_reputation = (int) nationPtr->reputation;
			max_reputation_nation_recno = i;
		}

		if( (int) nationPtr->kill_monster_score > max_kill_monster_score )
		{
			max_kill_monster_score = (int) nationPtr->kill_monster_score;
			max_kill_monster_nation_recno = i;
		}

		if( (int) nationPtr->overall_rating > max_overall_rating )
		{
			max_overall_rating = (int) nationPtr->overall_rating;
			max_overall_nation_recno = i;
		}
	}
}
//------- End of function NationArray::update_statistic -------//


//------- Begin of function NationArray::update_military_rating ------//
//
void NationArray::update_military_rating()
{
	int nationCombatLevelArray[MAX_NATION];

	memset( nationCombatLevelArray, 0, sizeof(nationCombatLevelArray) );

	//------ calculate firm statistic -------//

	Firm* firmPtr;

	int i;
	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->nation_recno == 0 ||
			 firmPtr->firm_id != FIRM_CAMP )
		{
			continue;
		}

		nationCombatLevelArray[firmPtr->nation_recno-1] +=
			((FirmCamp*)firmPtr)->total_combat_level() +
			((firmPtr->overseer_recno>0) + firmPtr->worker_count) * 20;		// 20 is the base military points for a unit, so the nation that has many more units can be reflected in the military rating
	}

	//------ calculate unit statistic -------//

	Unit* unitPtr;

	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno == 0 )
			continue;

		//---- if this unit is a ship, increase total_ship_combat_level ----//

		if( unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_SHIP )
		{
			nation_array[unitPtr->nation_recno]->total_ship_combat_level += (int) unitPtr->hit_points;
		}

		//----------------------------------//

		if( unitPtr->unit_mode == UNIT_MODE_OVERSEE )		// firm commanders are counted above with firm_array
			continue;

		int addPoints = (int) unitPtr->hit_points;

		UnitInfo* unitInfo = unit_res[unitPtr->unit_id];

		if( unitInfo->unit_class == UNIT_CLASS_WEAPON )
			addPoints += (unitInfo->weapon_power + unitPtr->get_weapon_version() - 1) * 30;

		if( unitPtr->leader_unit_recno && !unit_array.is_deleted(unitPtr->leader_unit_recno) )
			addPoints += addPoints * unit_array[unitPtr->leader_unit_recno]->skill.skill_level / 100;

		nationCombatLevelArray[unitPtr->nation_recno-1] += addPoints + 20;		// 20 is the base military points for a unit, so the nation that has many more units can be reflected in the military rating
	}

	//------ update nation statistic ------//

	for( i=size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		nation_array[i]->military_rating = nationCombatLevelArray[i-1]/50;
	}
}
//------- End of function NationArray::update_military_rating -------//


//------- Begin of function NationArray::update_total_human_count ------//
//
// Update total human count as there are some bugs in calculation of
// total human count. This function is used for on-fly correcting
// the problem.
//
void NationArray::update_total_human_count()
{
	int totalHumanCountArray[MAX_NATION];

	memset( totalHumanCountArray, 0, sizeof(totalHumanCountArray) );

	//------ calculate firm statistic -------//

	Firm* firmPtr;

	int i;
	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->nation_recno == 0 )
			continue;

		if( firmPtr->firm_id == FIRM_CAMP || firmPtr->firm_id == FIRM_BASE )
		{
			for( int j=firmPtr->worker_count-1 ; j>=0 ; j-- )
			{
				if( firmPtr->worker_array[j].race_id )
				{
					totalHumanCountArray[firmPtr->nation_recno-1]++;
				}
			}
		}
	}

	//------ calculate unit statistic -------//

	Unit* unitPtr;

	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_truly_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno && unitPtr->race_id &&
			 unitPtr->rank_id != RANK_KING )		// does not count kings
		{
			totalHumanCountArray[unitPtr->nation_recno-1]++;
		}
	}

	//------ update nation statistic ------//

	for( i=size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		#ifdef DEBUG
		err_when( nation_array[i]->total_human_count != totalHumanCountArray[i-1] );
		#else
			if( nation_array[i]->total_human_count != totalHumanCountArray[i-1] )
				nation_array[i]->total_human_count = totalHumanCountArray[i-1];
		#endif
	}
}
//------- End of function NationArray::update_total_human_count -------//


//-------- Begin of function NationArray::get_human_name --------//
//
// <int> nationNameId  - name id. of the nation
// [int] firstWordOnly - whether only return the first word of the name or not.
//								 (default: 0)
//
char* NationArray::get_human_name(int nationNameId, int firstWordOnly)
{
	int nationRecno = -nationNameId;

	err_when( nationRecno < 1 || nationRecno > MAX_NATION );

	static char humanNameOneWord[HUMAN_NAME_LEN+1];

	char* humanName = human_name_array[nationRecno-1];

	if( firstWordOnly )
	{
		int i;
		for( i=0 ; i<HUMAN_NAME_LEN && humanName[i] && humanName[i]!=' ' ; i++ )
			humanNameOneWord[i] = humanName[i];

		humanNameOneWord[i] = NULL;

		return humanNameOneWord;
	}
	else
	{
		return humanName;
	}
}
//--------- End of function NationArray::get_human_name ---------//


//-------- Begin of function NationArray::set_human_name --------//
//
void NationArray::set_human_name(int nationRecno, char* nameStr)
{
	err_when( nationRecno < 1 || nationRecno > MAX_NATION );

	strncpy( human_name_array[nationRecno-1], nameStr, HUMAN_NAME_LEN );
	human_name_array[nationRecno-1][HUMAN_NAME_LEN] = NULL;
}
//--------- End of function NationArray::set_human_name ---------//


//------- Begin of function NationArray::disp_nation_color ------//
//
// Display a nation color in a rectangular box.
//
void NationArray::disp_nation_color(int x, int y, int nationColor)
{
	vga.active_buf->bar( x, y-2, x+12, y+10, nationColor );
	vga.active_buf->rect( x, y-2, x+12, y+10, 1, nationColor+2 );
}
//------- End of function NationArray::disp_nation_color -------//


//------- Begin of function NationBase::should_attack ------//
//
char NationArray::should_attack(short attackingNation, short attackedNation)
{
	if( attackingNation == attackedNation )
		return 0;

	if( attackingNation == 0 || attackedNation == 0 )
		return 1;

	if( nation_array.is_deleted(attackingNation) || nation_array.is_deleted(attackedNation)  )
		return 0;

	return this->operator[](attackingNation)->get_relation_should_attack(attackedNation);
}
//------- End of function NationBase::should_attack -------//


#ifdef DEBUG

//------- Begin of function NationArray::operator[] -----//

Nation* NationArray::operator[](int recNo)
{
	Nation* nationPtr = (Nation*) get_ptr(recNo);

	if( !nationPtr )
		err.run( "NationArray[] is deleted" );

	return nationPtr;
}
//--------- End of function NationArray::operator[] ----//


//------- Begin of function NationArray::operator~ -----//

Nation* NationArray::operator~()
{
	if( !player_recno || !player_ptr )
		err.run( "~NationArray error" );

	return player_ptr;
}
//--------- End of function NationArray::operator~ ----//

#endif

//### begin alex 22/9 ###//
//--------- Begin of function NationArray::draw_profile ---------//
void NationArray::draw_profile()
{
#ifdef DEBUG
	static unsigned long lastDrawTime = m.get_time();

	if(m.get_time() >= lastDrawTime + 1000)
	{
		last_nation_ai_profile_time = nation_ai_profile_time;
		nation_ai_profile_time = 0L;
		last_nation_profile_time = nation_profile_time;
		nation_profile_time = 0L;
		lastDrawTime = m.get_time();
	}

	String str;
	str  = "Nation: ";
	font_news.disp( ZOOM_X1+10, ZOOM_Y1+30, str, MAP_X2);

	str = "";
	str += last_nation_ai_profile_time;
	font_news.disp( ZOOM_X1+60, ZOOM_Y1+30, str, MAP_X2);

	str = "";
	str += last_nation_profile_time;
	font_news.disp( ZOOM_X1+100, ZOOM_Y1+30, str, MAP_X2);
#endif	
}
//----------- End of function NationArray::draw_profile -----------//
//#### end alex 22/9 ####//
