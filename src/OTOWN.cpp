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

//cFilename    : OTOWN.CPP
//Description : Object Town

#include <string.h>
#include <ALL.h>
#include <OVGA.h>
#include <OWORLD.h>
#include <OSYS.h>
#include <OIMGRES.h>
#include <ONEWS.h>
#include <OCONFIG.h>
#include <OFONT.h>
#include <OINFO.h>
#include <OPLANT.h>
#include <ODATE.h>
#include <OSPY.h>
#include <OCONFIG.h>
#include <OF_MARK.h>
#include <OF_CAMP.h>
#include <OUNIT.h>
#include <OTERRAIN.h>
#include <OREBEL.h>
#include <OSPRTRES.h>
#include <ORACERES.h>
#include <ONATION.h>
#include <OREMOTE.h>
#include <OTOWN.h>
#include <OTownNetwork.h>
#include <ONEWS.h>
#include <OANLINE.h>
// ##### begin Gilbert 9/10 ######//
#include <OSE.h>
// ##### end Gilbert 9/10 ######//
#include <OSERES.h>
#include <OLOG.h>
#include <ConfigAdv.h>

static char random_race();


//--------- Begin of function Town::Town ----------//
//
Town::Town()
{
	memset( this, 0, sizeof(Town) );
}
//---------- End of function Town::Town ----------//


//--------- Begin of function Town::init ----------//
//
// <int> nationRecno - the nation recno
// <int> raceId 	   - the race of the majority of the town poulation
// <int> xLoc, yLoc  - location of the town
//
void Town::init(int nationRecno, int raceId, int xLoc, int yLoc)
{
	nation_recno = nationRecno;
	race_id		 = raceId;

	err_when( raceId<1 || raceId>MAX_RACE );

	//---- set the town section's absolute positions on the map ----//

	loc_x1 = xLoc;
	loc_y1 = yLoc;
	loc_x2 = loc_x1 + STD_TOWN_LOC_WIDTH - 1;
	loc_y2 = loc_y1 + STD_TOWN_LOC_HEIGHT - 1;

	abs_x1 = xLoc * ZOOM_LOC_WIDTH;
	abs_y1 = yLoc * ZOOM_LOC_HEIGHT;
	abs_x2 = abs_x1 + STD_TOWN_LOC_WIDTH * ZOOM_LOC_WIDTH  - 1;
	abs_y2 = abs_y1 + STD_TOWN_LOC_HEIGHT * ZOOM_LOC_HEIGHT - 1;

	center_x = (loc_x1+loc_x2)/2;
	center_y = (loc_y1+loc_y2)/2;

	region_id = world.get_region_id( center_x, center_y );

	ai_town = !nation_recno || nation_array[nation_recno]->nation_type == NATION_AI;    // nation_recno==0 for independent towns
	ai_link_checked = 1;			// check the linked towns and firms connected only if ai_link_checked==0

	independent_unit_join_nation_min_rating = 100 + misc.random(150);		// the minimum rating a nation must have in order for an independent unit to join it

	setup_date = info.game_date;

	//-------- init resistance ------------//

	if( nationRecno==0 )
	{
		for( int i=0 ; i<MAX_RACE ; i++ )
		{
			for( int j=0 ; j<MAX_NATION ; j++ )
			{
				race_resistance_array[i][j] = (float) 60 + misc.random(40);
				race_target_resistance_array[i][j] = -1;
			}
		}

		//--- some independent towns have higher than normal combat level for its defender ---//

		switch( config.independent_town_resistance )
		{
			case OPTION_LOW:
				town_combat_level = CITIZEN_COMBAT_LEVEL;
				break;

			case OPTION_MODERATE:
				town_combat_level = 10 + misc.random(20);
				break;

			case OPTION_HIGH:
				town_combat_level = 10 + misc.random(30);
				if( misc.random(5)==0 )
					town_combat_level += misc.random(30);
				break;

			default:
				err_here();
		}
	}

	//-------------------------------------//

	town_name_id = town_res.get_new_name_id(raceId);

	set_world_matrix();

	setup_link();

	//-------- if this is an AI town ------//

	if( ai_town && nation_recno )
	{
		nation_array[nation_recno]->add_town_info(town_recno);

		update_base_town_status();
	}

	//------ set national auto policy -----//

	if( nation_recno )
	{
		Nation* nationPtr = nation_array[nation_recno];

		set_auto_collect_tax_loyalty( nationPtr->auto_collect_tax_loyalty );
		set_auto_grant_loyalty( nationPtr->auto_grant_loyalty );
	}

	//--------- setup town network ---------//

	town_network_pulsed = false;
	town_network_recno = town_network_array.town_created(town_recno, nation_recno, linked_town_array, linked_town_count);
}
//--------- End of function Town::init ----------//


//--------- Begin of function Town::deinit ----------//
//
void Town::deinit()
{
	if( !town_recno )
		return;

	//--------- remove from town network ---------//

	town_network_array.town_destroyed(town_recno);


	clear_defense_mode();

	//------- if it's an AI town --------//

	if( ai_town && nation_recno )
		nation_array[nation_recno]->del_town_info(town_recno);

	//------ if this town is the nation's larget town, reset it ----//

	if( nation_recno &&
		 nation_array[nation_recno]->largest_town_recno == town_recno )
	{
		nation_array[nation_recno]->largest_town_recno = 0;
	}

	//-----------------------------------//

	restore_world_matrix();

	release_link();

	//-- if there is a unit being trained when the town vanishes --//

	if(train_unit_recno)
		unit_array.disappear_in_town(train_unit_recno, town_recno);

	//------- if the current town is the selected -----//

	if( town_array.selected_recno == town_recno )
	{
		town_array.selected_recno = 0;
		info.disp();
	}

	//------- reset parameters ---------//

	town_recno = 0;
	town_network_recno = 0;
}
//--------- End of function Town::deinit ----------//


//------- Begin of function Town::set_world_matrix --------//
//
// Set the cargo id of current town section on the world matrix
//
void Town::set_world_matrix()
{
	//--- if a nation set up a town in a location that the player has explored, contact between the nation and the player is established ---//

	int xLoc, yLoc;
	Location* locPtr;

	for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
	{
		for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
		{
			locPtr = world.get_loc(xLoc, yLoc);

			if(!locPtr->cargo_recno)	// skip the location where the settle unit is standing
				locPtr->set_town(town_recno);
		}
	}

	//--- if a nation set up a town in a location that the player has explored, contact between the nation and the player is established ---//

	establish_contact_with_player();

	//---- set this town's influence on the map ----//

	if( nation_recno )
		world.set_power(loc_x1, loc_y1, loc_x2, loc_y2, nation_recno);

	//------------ reveal new land ----------//

	if( nation_recno == nation_array.player_recno ||
		 ( nation_recno && nation_array[nation_recno]->is_allied_with_player ) )
	{
		world.unveil( loc_x1, loc_y1, loc_x2, loc_y2 );
		world.visit( loc_x1, loc_y1, loc_x2, loc_y2, EXPLORE_RANGE-1 );
	}

	//---- if the newly built firm is visual in the zoom window, redraw the zoom buffer ----//

	if( is_in_zoom_win() )
		sys.zoom_need_redraw = 1;  // set the flag on so it will be redrawn in the next frame
}
//-------- End of function Town::set_world_matrix --------//


//------- Begin of function Town::establish_contact_with_player --------//
//
// See if the town's location is an explored area, establish contact
// with the player.
//
void Town::establish_contact_with_player()
{
	if( !nation_recno )
		return;

	//--- if a nation set up a town in a location that the player has explored, contact between the nation and the player is established ---//

	int xLoc, yLoc;
	Location* locPtr;

	for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
	{
		for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
		{
			locPtr = world.get_loc(xLoc, yLoc);

			if( locPtr->explored() && nation_array.player_recno )
			{
				NationRelation *relation = (~nation_array)->get_relation(nation_recno);

				if( !remote.is_enable() )
				{
					relation->has_contact = 1;
				}
				else
				{
					if( !relation->contact_msg_flag && !relation->has_contact )
					{
						// packet structure : <player nation> <explored nation>
						short *shortPtr = (short *)remote.new_send_queue_msg(MSG_NATION_CONTACT, 2*sizeof(short));
						*shortPtr = nation_array.player_recno;
						shortPtr[1] = nation_recno;
						relation->contact_msg_flag = 1;
					}
				}
			}
		}
	}
}
//-------- End of function Town::establish_contact_with_player --------//


//--------- Begin of function Town::restore_world_matrix --------//
//
// When the town section is destroyed, restore the original land id
//
void Town::restore_world_matrix()
{
	int xLoc, yLoc;

	for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
	{
		for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
      {
			world.get_loc(xLoc,yLoc)->remove_town();
		}
	}

	//---- restore this town's influence on the map ----//

	if( nation_recno )
		world.restore_power(loc_x1, loc_y1, loc_x2, loc_y2, town_recno, 0);

	//---- if the newly built firm is visual in the zoom window, redraw the zoom buffer ----//

	if( is_in_zoom_win() )
		sys.zoom_need_redraw = 1;
}
//----------- End of function Town::restore_world_matrix --------//


//---------- Begin of function Town::next_day --------//
//
void Town::next_day()
{
	err_when( population>MAX_TOWN_POPULATION );
	err_when( town_recno<1 );
	err_when( town_recno>town_array.size() );

	int townRecno = town_recno;

	//------ update quality_of_life --------//

	update_quality_of_life();

	//---------- update population ----------//

#if defined(DEBUG) && defined(ENABLE_LOG)
	String logStr;
	logStr = "begin Town::next_day, town_recno = ";
	logStr += townRecno;
	logStr += " nation=";
	logStr += nation_recno;
	LOG_MSG(logStr);
#endif

	if( info.game_date%30 == town_recno%30 )
	{
		LOG_MSG(" population_grow");
		population_grow();
		LOG_MSG(misc.get_random_seed());
	}

	err_when(population>MAX_TOWN_POPULATION);

	//------- update link status to camps ------//

	LOG_MSG(" update_camp_link");
	update_camp_link();
	LOG_MSG(misc.get_random_seed());

	//------ update target loyalty/resistance -------//

	if( info.game_date%15 == town_recno%15 )
	{
		if( nation_recno )
		{
			LOG_MSG(" update_target_loyalty");
			update_target_loyalty();
		}
		else
		{
			LOG_MSG(" update_target_resistance");
			update_target_resistance();		// update resistance for independent towns
		}
		LOG_MSG(misc.get_random_seed());
	}

	//------ update loyalty/resistance -------//

	if( info.game_date%5 == town_recno%5 )
	{
		if( nation_recno )
		{
			LOG_MSG(" update_target_loyalty");
			update_loyalty();
		}
		else
		{
			LOG_MSG(" update_target_resistance");
			update_resistance();
		}
		LOG_MSG(misc.get_random_seed());

		if( town_array.is_deleted(townRecno) )
			return;
	}

	//------ think town people migration -------//

	if( config_adv.town_migration && info.game_date%15 == town_recno%15 )
	{
		LOG_MSG(" think_migrate");
		think_migrate();
		LOG_MSG(misc.get_random_seed());

		if( town_array.is_deleted(townRecno) )
			return;
	}

	//-------- think about rebel -----//

	if( nation_recno && info.game_date%15 == town_recno%15 )
	{
		LOG_MSG(" think_rebel");
		think_rebel();
		LOG_MSG(misc.get_random_seed() );

		if( town_array.is_deleted(townRecno) )
			return;
	}

	//-------- think about surrender -----//

	if( nation_recno &&
		 ( info.game_date%15==town_recno%15 || average_loyalty()==0 ) )
	{
		LOG_MSG(" think_surrender");
		think_surrender();		// for nation town only, independent town surrender is handled in update_resistance()
		LOG_MSG(misc.get_random_seed() );

		if( town_array.is_deleted(townRecno) )
			return;
	}

	//------- process training -------//

	//### begin alex 6/9 ###//
	/*if( nation_recno && train_unit_recno )
	{
		process_train();

		if( town_array.is_deleted(townRecno) )	// when the last peasant in the town is trained, the town disappear
			return;
	}*/
	if(nation_recno)
	{
		if(train_unit_recno)
		{
			LOG_MSG(" process_train");
			process_train();
			LOG_MSG(misc.get_random_seed() );
		}
		else
		{
			LOG_MSG(" process_queue");
			process_queue();
			LOG_MSG(misc.get_random_seed() );
		}

		if( town_array.is_deleted(townRecno) )	// when the last peasant in the town is trained, the town disappear
			return;
	}
	//#### end alex 6/9 ####//

	//-------- process food ---------//

	if( nation_recno )
	{
		LOG_MSG(" process_food");
		process_food();
		LOG_MSG(misc.get_random_seed());

		if( town_array.is_deleted(townRecno) )
			return;
	}

	//------ auto collect tax and auto grant -------//

	if( nation_recno )
	{
		LOG_MSG(" process_auto");
		process_auto();
		LOG_MSG(misc.get_random_seed() );

		if( town_array.is_deleted(townRecno) )
			return;
	}

	//------ collect yearly tax -------//

	if( nation_recno && info.game_month==1 && info.game_day==1 )
	{
		collect_yearly_tax();
	}

	//------ catching spies -------//

	LOG_MSG(" process_auto");

	if( info.game_date%30 == town_recno%30 )
		spy_array.catch_spy(SPY_TOWN, town_recno);

	LOG_MSG(misc.get_random_seed() );

	if( town_array.is_deleted(townRecno) )
		return;

	//-------- update visibility ---------//

	if( nation_recno == nation_array.player_recno ||
		 (nation_recno && nation_array[nation_recno]->is_allied_with_player) )
	{
		world.visit( loc_x1, loc_y1, loc_x2, loc_y2, EXPLORE_RANGE-1 );
	}

	//--- recheck no_neighbor_space after a period, there may be new space available now ---//

	if( no_neighbor_space && info.game_date%180 == town_recno%180 )
	{
		LOG_MSG("begin finding neighor space");
		short buildXLoc, buildYLoc;

		//--- for independent town, since we can't call find_best_firm_loc(), we just set no_neighbor_space to 0 every 6 months, if it still has no space, then no_neighbor_space will be set 1 again. ---//

		if( nation_recno==0 || nation_array[nation_recno]->find_best_firm_loc(FIRM_INN, loc_x1, loc_y1, buildXLoc, buildYLoc) )		// whether it's FIRM_INN or not really doesn't matter, just any firm type will do
			no_neighbor_space = 0;

		LOG_MSG(misc.get_random_seed());
	}

	//------ decrease penalties -----//

	if( accumulated_collect_tax_penalty > 0 )
		accumulated_collect_tax_penalty--;

	if( accumulated_reward_penalty > 0 )
		accumulated_reward_penalty--;

	if( accumulated_recruit_penalty > 0 )
		accumulated_recruit_penalty--;

	if( accumulated_enemy_grant_penalty > 0 )
		accumulated_enemy_grant_penalty--;

	//------------------------------------------------------------//
	// check for population for each town
	//------------------------------------------------------------//

   //err_when( town_array.is_deleted(townRecno) );
	if( town_array.is_deleted(townRecno) )
		return;

#ifdef DEBUG
	Firm	*firmPtr;
	int	i, j;
	short	pop[MAX_RACE];

	memset(pop, 0, sizeof(short)*MAX_RACE);
	for(i=linked_firm_count-1; i>=0; i--)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));
		firmPtr = firm_array[linked_firm_array[i]];

		if(firmPtr->overseer_recno)
		{
			if(firmPtr->overseer_town_recno==town_recno)
			{
				Unit *unitPtr = unit_array[firmPtr->overseer_recno];
				pop[unitPtr->race_id-1]++;
			}
		}

		if(firmPtr->worker_count)
		{
			for(j=firmPtr->worker_count-1; j>=0; j--)
			{
				if((firmPtr->worker_array[j]).town_recno==town_recno)
					pop[(firmPtr->worker_array[j]).race_id-1]++;
			}
		}
	}

	for(i=0; i<MAX_RACE; i++)
		err_when(pop[i]+jobless_race_pop_array[i] != race_pop_array[i]);
#endif

	//------------------------------------------------------------//
	// check the spy count 
	//------------------------------------------------------------//

#ifdef DEBUG
	short	raceSpyCountArray[MAX_RACE];		  

	memset( raceSpyCountArray, 0, sizeof(raceSpyCountArray) );

	for( i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		Spy* spyPtr = spy_array[i];

		if( spyPtr->spy_place == SPY_TOWN && spyPtr->spy_place_para == town_recno )
			raceSpyCountArray[spyPtr->race_id-1]++;
	}

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		err_when( race_spy_count_array[i] != raceSpyCountArray[i] );

		err_when( race_spy_count_array[i] > jobless_race_pop_array[i] );
	}

	town_res.get_name(town_name_id);
#endif
}
//----------- End of function Town::next_day ---------//


//---------- Begin of function Town::process_food --------//
//
void Town::process_food()
{
	//--------- Peasants produce food ---------//

	nation_array[nation_recno]->add_food( (float) jobless_population * PEASANT_FOOD_YEAR_PRODUCTION / 365 );

	//---------- People consume food ----------//

	nation_array[nation_recno]->consume_food( (float) population * PERSON_FOOD_YEAR_CONSUMPTION / 365 );
}
//----------- End of function Town::process_food ---------//


//---------- Begin of function Town::process_auto --------//
//
void Town::process_auto()
{
	if( !has_linked_own_camp )		// can only collect or grant when there is a linked camp of its own.
		return;

	Nation* nationPtr = nation_array[nation_recno];

	//----- auto collect tax -----//

	if( auto_collect_tax_loyalty > 0 )
	{
		if( accumulated_collect_tax_penalty == 0 &&
			 average_loyalty() >= auto_collect_tax_loyalty )
		{
			collect_tax(COMMAND_AI);
		}
	}

	//---------- auto grant -----------//

	if( auto_grant_loyalty > 0 )
	{
		if( accumulated_reward_penalty == 0 &&
			 average_loyalty() < auto_grant_loyalty && nationPtr->cash > 0 )
		{
			reward(COMMAND_AI);
		}
	}
}
//----------- End of function Town::process_auto ---------//


//---------- Begin of function Town::set_nation --------//
//
// Set the nation of this town.
//
// Possible changes:
//
// independent town -> nation town
// nation A's town -> nation B's town (surrender)
//
void Town::set_nation(int newNationRecno)
{
	if( nation_recno == newNationRecno )
		return;

	//--------- update town network (pre-step) ---------//
	town_network_array.town_pre_changing_nation(town_recno);


	clear_defense_mode();

	//------------- stop all actions to attack this town ------------//
	unit_array.stop_attack_town(town_recno);
	rebel_array.stop_attack_town(town_recno);

	//--------- update AI town info ---------//

	if( ai_town && nation_recno )
	{
		nation_array[nation_recno]->del_town_info(town_recno);
	}

	//--------- reset vars ---------//

	is_base_town = 0;
	town_defender_count = 0; 	// reset defender count

	//----- set power region of the new nation ------//

	world.restore_power(loc_x1, loc_y1, loc_x2, loc_y2, town_recno, 0);		// restore power of the old nation
	world.set_power(loc_x1, loc_y1, loc_x2, loc_y2, newNationRecno);			// set power of the new nation

	//--- update the cloaked_nation_recno of all spies in the firm ---//

	spy_array.change_cloaked_nation(SPY_TOWN, town_recno, nation_recno, newNationRecno);		// check the cloaked nation recno of all spies in the firm

	//--------- set nation_recno --------//

	int oldNationRecno = nation_recno;
	nation_recno = newNationRecno;

	if( nation_recno )      // reset rebel_recno if the town is then ruled by a nation
	{
		if( rebel_recno )
		{
			Rebel* rebelPtr = rebel_array[rebel_recno];

			err_when( rebelPtr->mobile_rebel_count > 0 );	// change nation shouldn't happen if mobile_rebel_count > 0

			rebel_array.del_rebel(rebel_recno);		// delete the rebel group
			rebel_recno = 0;
		}
	}

	//--------- update ai_town ----------//

	ai_town = 0;

	if( nation_recno==0 )		// independent town
	{
		ai_town = 1;
	}
	else if( nation_array[nation_recno]->nation_type==NATION_AI )
	{
		ai_town = 1;
		nation_array[nation_recno]->add_town_info(town_recno);
	}

	//------ set the loyalty of the town people ------//

	int i, nationRaceId;

	if( nation_recno )
		nationRaceId = nation_array[nation_recno]->race_id;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( nation_recno == 0 )			// independent town set up by rebel
			race_loyalty_array[i] = (float) 80;			// this affect the no. of defender if you attack the independent town
		else
		{
			if( nationRaceId == i+1 )
				race_loyalty_array[i] = (float) 40;    // loyalty is higher if the ruler and the people are the same race
			else
				race_loyalty_array[i] = (float) 30;
		}
	}

	//------- reset town_combat_level -------//

	town_combat_level = 0;

	//------ reset accumulated penalty ------//

	accumulated_collect_tax_penalty = 0;
	accumulated_reward_penalty	 = 0;
	accumulated_enemy_grant_penalty = 0;
	accumulated_recruit_penalty = 0;

	//---- if there is unit being trained currently, change its nation ---//

	if( train_unit_recno )
		unit_array[train_unit_recno]->change_nation(newNationRecno);

	//-------- update loyalty ---------//

	update_target_loyalty();

	//--- if a nation set up a town in a location that the player has explored, contact between the nation and the player is established ---//

	establish_contact_with_player();

	//----- if an AI nation took over this town, see if the AI can capture all firms linked to this town ----//

	if( nation_recno && nation_array[nation_recno]->is_ai() )
		think_capture_linked_firm();

	//--- when a town change nation, call the AI function of call linked firms ---//

	Firm* firmPtr;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->firm_ai )		// tell linked firms that this town has changed nation
			firmPtr->think_linked_town_change_nation(town_recno, oldNationRecno, newNationRecno);
	}

	//------ set national auto policy -----//

	if( nation_recno )
	{
		Nation* nationPtr = nation_array[nation_recno];

		set_auto_collect_tax_loyalty( nationPtr->auto_collect_tax_loyalty );
		set_auto_grant_loyalty( nationPtr->auto_grant_loyalty );
	}

	//---- reset the action mode of all spies in this town ----//

	spy_array.set_action_mode( SPY_TOWN, town_recno, SPY_IDLE );      // we need to reset it. e.g. when we have captured an enemy town, SPY_SOW_DISSENT action must be reset to SPY_IDLE

	//--------- update town network (post-step) ---------//
	town_network_array.town_post_changing_nation(town_recno, newNationRecno);

	//-------- refresh display ----------//

	if( town_array.selected_recno == town_recno )
		info.disp();
}
//----------- End of function Town::set_nation ---------//


//------ Begin of function Town::set_hostile_nation -------//
void Town::set_hostile_nation(int nationRecno)
{
	if(nationRecno==0)
		return;

	err_when(nationRecno>7); // only 8 bits
	independ_town_nation_relation |= (0x1 << nationRecno);
}
//----------- End of function Town::set_hostile_nation ---------//


//------ Begin of function Town::reset_hostile_nation -------//
void Town::reset_hostile_nation(int nationRecno)
{
	if(nationRecno==0)
		return;

	err_when(nationRecno>7); // only 8 bits
	independ_town_nation_relation &= ~(0x1 << nationRecno);
}
//----------- End of function Town::reset_hostile_nation ---------//


//------ Begin of function Town::is_hostile_nation -------//
// return 1 for hostile nation
// return 0 otherwise
//
int Town::is_hostile_nation(int nationRecno)
{
	if(nationRecno==0)
		return 0;

	err_when(nationRecno>7); // only 8 bits
	return (independ_town_nation_relation & (0x1 << nationRecno));
}
//----------- End of function Town::is_hostile_nation ---------//


//--------- Begin of function Town::init_pop ----------//
//
// Initialize population of the town.
//
// <int> raceId  - the id. of the race to add.
// <int> addPop  - the no. of people to add.
// <int> addLoyalty - loyalty of the people to add to the town
// [int] hasJob	  - whether the units will have jobs or not after moving into this town
//							 (default: 0)
// [int] firstInit - first initialization at the beginning of the game
//						   (default: 0)
//
void Town::init_pop(int raceId, int addPop, int addLoyalty, int hasJob, int firstInit)
{
	if(population>=MAX_TOWN_POPULATION)
		return;

	int addPopulation = MIN(addPop, MAX_TOWN_POPULATION-population);
	
	//-------- update population ---------//

	race_pop_array[raceId-1] += addPopulation;
	population               += addPopulation;

	err_when( race_pop_array[raceId-1] > MAX_TOWN_POPULATION );
	err_when( population               > MAX_TOWN_POPULATION );

	if( !hasJob )
	{
		jobless_population               += addPopulation;
		jobless_race_pop_array[raceId-1] += addPopulation;
	}

	err_when( jobless_race_pop_array[raceId-1] > MAX_TOWN_POPULATION );
	err_when( jobless_population               > MAX_TOWN_POPULATION );

	//------- update loyalty --------//

	err_when( race_loyalty_array[raceId-1]<0 || race_loyalty_array[raceId-1]>100 );

	if( firstInit )	// first initialization at the beginning of the game
	{
		if( nation_recno )
		{
			race_loyalty_array[raceId-1] = (float) addLoyalty;
			race_target_loyalty_array[raceId-1] = addLoyalty;
		}
		else
		{
			for( int j=0 ; j<MAX_NATION ; j++)							// reset resistance for non-existing races
			{
				race_resistance_array[raceId-1][j] = (float) addLoyalty;
				race_target_resistance_array[raceId-1][j] = addLoyalty;
			}
		}
	}
	else
	{
		if( nation_recno )
		{
			race_loyalty_array[raceId-1] =
					  ( race_loyalty_array[raceId-1] * (race_pop_array[raceId-1]-addPopulation)
					  + (float) addLoyalty * addPopulation ) / race_pop_array[raceId-1];

			race_target_loyalty_array[raceId-1] = (char) race_loyalty_array[raceId-1];
		}
		else
		{
			for( int j=0 ; j<MAX_NATION ; j++)							// reset resistance for non-existing races
			{
				race_resistance_array[raceId-1][j] =
					  ( race_resistance_array[raceId-1][j] * (race_pop_array[raceId-1]-addPopulation)
					  + (float) addLoyalty * addPopulation ) / race_pop_array[raceId-1];

				race_target_resistance_array[raceId-1][j] = (char) race_resistance_array[raceId-1][j];
			}
		}
	}

	err_when( addLoyalty<0 || addLoyalty>100 );
	err_when( race_loyalty_array[raceId-1]<0 || race_loyalty_array[raceId-1]>100 );
	err_when( race_target_loyalty_array[raceId-1]<0 || race_target_loyalty_array[raceId-1]>100 );

	//---------- update town parameters ----------//

	town_array.distribute_demand();

	if( nation_recno )
		update_loyalty();
	else
		update_resistance();
}
//--------- End of function Town::init_pop ----------//


//--------- Begin of function Town::inc_pop -------//
//
// <int> raceId      - the race id. of the population
// <int> unitHasJob  - whether the unit will get a job after moving into the town
// <int> unitLoyalty - loyalty of the unit.
//
void Town::inc_pop(int raceId, int unitHasJob, int unitLoyalty)
{
	err_when( unitLoyalty < 0 );

	//---------- increase population ----------//

	population++;
	race_pop_array[raceId-1]++;

	err_when( race_pop_array[raceId-1] > MAX_TOWN_POPULATION );
	err_when( population               > MAX_TOWN_POPULATION );

	if( !unitHasJob )
	{
		jobless_population++;
		jobless_race_pop_array[raceId-1]++;

		err_when( jobless_race_pop_array[raceId-1] > MAX_TOWN_POPULATION );
		err_when( jobless_population               > MAX_TOWN_POPULATION );
	}

	//------- update loyalty --------//

	if( nation_recno )		// if the unit has an unit
	{
		err_when( race_loyalty_array[raceId-1]<0 || race_loyalty_array[raceId-1]>100 );

		race_loyalty_array[raceId-1] =
				  ( race_loyalty_array[raceId-1] * (race_pop_array[raceId-1]-1)
				  + (float) unitLoyalty ) / race_pop_array[raceId-1];

		err_when( unitLoyalty<0 || unitLoyalty>100 );
		err_when( race_loyalty_array[raceId-1]<0 || race_loyalty_array[raceId-1]>100 );
	}

	//-- if the race's population exceeds the capacity of the town layout --//

	if( race_pop_array[raceId-1] > max_race_pop_array[raceId-1] )
	{
		auto_set_layout();
	}
}
//---------- End of function Town::inc_pop --------//


//--------- Begin of function Town::dec_pop -------//
//
// <int> raceId      - the race id. of the population
// <int> unitHasJob  - whether the unit current has a job or not
//
void Town::dec_pop(int raceId, int unitHasJob)
{
	population--;
	race_pop_array[raceId-1]--;

	err_when( population < 0 );
	err_when( race_pop_array[raceId-1] < 0 );

	if( !unitHasJob )
	{
		jobless_population--;
		jobless_race_pop_array[raceId-1]--;

		err_when( jobless_population < 0 );
		err_when( jobless_race_pop_array[raceId-1] < 0 );
	}

	err_when( race_pop_array[raceId-1] < jobless_race_pop_array[raceId-1] );

	//------- if all the population are gone --------//

	if( population==0 )		// it will be deleted in TownArray::process()
	{
		if( nation_recno == nation_array.player_recno )
	   		news_array.town_abandoned(town_recno);

		deinit();
		return;
	}

	//-- if the race's population drops to too low, change the town layout --//

	if( race_pop_array[raceId-1] <= max_race_pop_array[raceId-1]-POPULATION_PER_HOUSE )
	{
		auto_set_layout();
	}
}
//---------- End of function Town::dec_pop --------//


//--------- Begin of function Town::population_grow --------//
//
void Town::population_grow()
{
	if( town_defender_count )
		return;

	if( population >= MAX_TOWN_GROWTH_POPULATION || population >= MAX_TOWN_POPULATION )
		return;

	//-------- population growth by birth ---------//

	#ifdef DEBUG
		short	debugPopulation = population;
		char debugRaceGrowth[MAX_RACE] = {0};
	#endif

	int i, autoSetFlag=0;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		//-- the population growth in an independent town is slower than in a nation town ---//

		if( nation_recno )
			race_pop_growth_array[i] += race_pop_array[i] * (100+quality_of_life) / 100;
		else
			race_pop_growth_array[i] += race_pop_array[i];

		int loopCount=0;

		while( race_pop_growth_array[i] > 100 )
		{
			err_when( loopCount++ > 100 );

			race_pop_growth_array[i] -= 100;

			race_pop_array[i]++;
			jobless_race_pop_array[i]++;

			population++;
			jobless_population++;

			err_when( race_pop_array[i] 			> MAX_TOWN_POPULATION );
			err_when( jobless_race_pop_array[i] > MAX_TOWN_POPULATION );

			err_when( population > MAX_TOWN_POPULATION );
			err_when( jobless_population > MAX_TOWN_POPULATION );

			#ifdef DEBUG
				debugRaceGrowth[i]++;
			#endif

			//-- if the race's population grows too high, change the town layout --//

			if( race_pop_array[i] > max_race_pop_array[i] )
			{
				autoSetFlag = 1;
			}

			if(population>=MAX_TOWN_POPULATION)
				break;
		}

		if(population>=MAX_TOWN_POPULATION)
			break;
	}

	if( autoSetFlag )
		auto_set_layout();
}
//----------- End of function Town::population_grow --------//


//-------- Begin of function Town::race_harmony --------//
//
// Return a harmony rating of the given race.
//
// <int> raceId - id. of the race
//
int Town::race_harmony(int raceId)
{
	if( population==0 )
		return 0;
	else
		return 100 * race_pop_array[raceId-1] / population;
}
//--------- End of function Town::race_harmony ---------//


//---------- Begin of function Town::update_target_loyalty --------//
//
// Update loyalty for nation towns.
//
void Town::update_target_loyalty()
{
	if( !nation_recno )		// return if independent towns
		return;

	//----- update loyalty of individual races -------//
	//
	// Loyalty is determined by:
	//
	// - residential harmony
	// - whether the people are racially homogeneous to the king
	// - the nation's reputation
	// - command bases overseeing the town.
	// - quality of life
	// - employment rate
	//
	// Quality of life is determined by:
	//
	// - The provision of goods to the villagers. A more constant
	//	  supply and a bigger variety of goods give to high quality of life.
	//
	//------------------------------------------------//

	//------- set target loyalty of each race --------//

	Nation* nationPtr = nation_array[nation_recno];
	int 	  i, targetLoyalty;
	int 	  nationRaceId = nationPtr->race_id;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] == 0 )
			continue;

		//------- calculate the target loyalty -------//

		targetLoyalty = race_harmony(i+1)/3 +				// 0 to 33
							 (int)nationPtr->reputation/4;	// -25 to +25

		//---- employment help increase loyalty ----//

		targetLoyalty += 30 - 30 * jobless_race_pop_array[i] / race_pop_array[i];		// +0 to +30

		if( race_res.is_same_race(i+1, nationRaceId) )
			targetLoyalty += 10;

		if( targetLoyalty < 0 )		// targetLoyalty can be negative if there are hostile races conflicts
			targetLoyalty = 0;

		if( targetLoyalty > 100 )
			targetLoyalty = 100;

		//----------------------------------------//

		race_target_loyalty_array[i] = targetLoyalty;

		err_when( targetLoyalty<0 || targetLoyalty>100 );
	}

	//----- process command bases that have influence on this town -----//

	int     baseInfluence, thisInfluence, commanderRaceId;
	Firm*   firmPtr;
	Nation  *baseNationPtr;
	Unit*   unitPtr;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_enable_array[i] != LINK_EE )
			continue;

		firmPtr = firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id!=FIRM_CAMP || !firmPtr->overseer_recno )
			continue;

		//-------- get nation and commander info ------------//

		unitPtr = unit_array[firmPtr->overseer_recno];
		commanderRaceId = unitPtr->race_id;

		baseNationPtr = nation_array[firmPtr->nation_recno];

		//------ if this race is the overseer's race -------//

		baseInfluence = unitPtr->skill.get_skill(SKILL_LEADING)/3;		// 0 to 33

		if( unitPtr->rank_id == RANK_KING )			// 20 points bonus for king
			baseInfluence += 20;

		//------------ update all race -----------//

		for( int j=0 ; j<MAX_RACE ; j++ )
		{
			if( !race_pop_array[j] )
				continue;

			//---- if the overseer's race is the same as this race ---//

			thisInfluence = baseInfluence;

			if( unitPtr->race_id == j+1 )
				thisInfluence += 8;

			//--- if the overseer's nation's race is the same as this race ---//

			if( baseNationPtr->race_id == j+1 )
				thisInfluence += 8;

			//------------------------------------------//

			if( firmPtr->nation_recno == nation_recno )	// if the command base belongs to the same nation
			{
				targetLoyalty = race_target_loyalty_array[j] + thisInfluence;
				race_target_loyalty_array[j] = MIN(100, targetLoyalty);
			}
			else if( unitPtr->race_id == j+1 )		// for enemy camps, only decrease same race peasants
			{
				targetLoyalty = race_target_loyalty_array[j] - thisInfluence;
				race_target_loyalty_array[j] = MAX(0, targetLoyalty);
			}
		}
	}

	//------- apply quality of life -------//

	int qolContribution = config_adv.town_loyalty_qol ?
		(quality_of_life-50)/3 :			// -17 to +17
		0;						// off
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] == 0 )
			continue;

		targetLoyalty = race_target_loyalty_array[i];

		// Quality of life only applies to the part above 30 loyalty
		if (targetLoyalty > 30)
		{
			targetLoyalty = MAX(30, targetLoyalty + qolContribution);
			race_target_loyalty_array[i] = MIN(100, targetLoyalty);
		}
	}

	//------- update link status to linked enemy camps -------//

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id != FIRM_CAMP )
			continue;

		//------------------------------------------//
		// If this town is linked to a own camp,
		// disable all links to enemy camps, otherwise
		// enable all links to enemy camps.
		//------------------------------------------//

		if( firmPtr->nation_recno != nation_recno )
			toggle_firm_link( i+1, !has_linked_own_camp, COMMAND_AUTO );
	}	
}
//----------- End of function Town::update_target_loyalty ---------//


//------ Begin of function Town::update_loyalty -----//
//
// Update loyalty and resistance towards the target.
//
void Town::update_loyalty()
{
	if( !nation_recno )
		return;

	//------------- update loyalty -------------//

	float targetLoyalty, loyaltyInc, loyaltyDec;

	for(int i=0; i<MAX_RACE; i++)
	{
		if( race_pop_array[i] == 0 )
			continue;

		targetLoyalty = (float) race_target_loyalty_array[i];

		if(race_loyalty_array[i] < targetLoyalty)
		{
			//--- if this town is linked to enemy camps, but no own camps, no increase in loyalty, only decrease ---//

			if( !has_linked_own_camp && has_linked_enemy_camp )
				continue;

			//-------------------------------------//

			loyaltyInc = (targetLoyalty-race_loyalty_array[i]) / 30;

			change_loyalty( i+1, MAX(loyaltyInc, (float)0.5) );
		}
		else if(race_loyalty_array[i] > targetLoyalty)
		{
			loyaltyDec = (race_loyalty_array[i]-targetLoyalty) / 30;

			change_loyalty( i+1, -MAX(loyaltyDec, (float)0.5) );
		}
	}
}
//------- End of function Town::update_loyalty ------//


//------ Begin of function Town::change_loyalty -----//
//
// <int>   raceId		    - id. of the race to be changed in loyalty.
// <float> loyaltyChange - the amount of change in loyalty.
//
void Town::change_loyalty(int raceId, float loyaltyChange)
{
	float newLoyalty = race_loyalty_array[raceId-1] + loyaltyChange;

   newLoyalty = MIN( 100, newLoyalty );
	newLoyalty = MAX(   0, newLoyalty );

	race_loyalty_array[raceId-1] = newLoyalty;
}
//------- End of function Town::change_loyalty ------//


//------ Begin of function Town::change_resistance -----//
//
// <int>   raceId		    	 - id. of the race to be changed in loyalty.
// <int>   nationRecno   	 - recno of the nation which the resistance is towards
// <float> resistanceChange - the amount of change in resistance.
//
void Town::change_resistance(int raceId, int nationRecno, float resistanceChange)
{
	err_when( raceId < 1 || raceId > MAX_RACE );
	err_when( nation_array.is_deleted(nationRecno) );

	float newResistance = race_resistance_array[raceId-1][nationRecno-1] + resistanceChange;

	newResistance = MIN( 100, newResistance );
	newResistance = MAX(   0, newResistance );

	race_resistance_array[raceId-1][nationRecno-1] = newResistance;
}
//------- End of function Town::change_resistance ------//


//------ Begin of function Town::update_target_resistance ------//
//
// Influence from a command base to a town (simplied version)
// ----------------------------------------------------------
// -Reputation of the nation
// -Race of the general and the king (whether its race is the same as the town people's race)
// -Leadership of the general
//
// If the conqueror's race id is the same as the town people's race id.
//   Influence = reputation/2
// else
//   Influence = reputation/4
//
// If the general's race is the same as the town people's race,
//   Influence += general leadership/2
// else
//   Influence += general leadership/4
//
void Town::update_target_resistance()
{
	int     i, j;
	Firm*   firmPtr;
	Unit*   unitPtr;

	if( population==0 || linked_firm_count==0 )
		return;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		for( j=0 ; j<MAX_NATION ; j++ )
		{
			race_target_resistance_array[i][j] = -1;		// -1 means influence is not present
		}
	}

	//---- if this town is controlled by rebels, no decrease in resistance ----//

	if( rebel_recno )
		return;

	//----- count the command base that has influence on this town -----//

	int curValue, newValue;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_enable_array[i] != LINK_EE )
			continue;

		firmPtr = firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id!=FIRM_CAMP || !firmPtr->overseer_recno )
			continue;

		//-------- get nation and commander info ------------//

		unitPtr = unit_array[firmPtr->overseer_recno];

		curValue = race_target_resistance_array[unitPtr->race_id-1][unitPtr->nation_recno-1];
		newValue = 100-camp_influence(firmPtr->overseer_recno);

		err_when( newValue < 0 );

		if( curValue == -1 || newValue < curValue )		// need to do this comparsion as there may be more than one command bases of the same nation linked to this town, we use the one with the most influence.
			race_target_resistance_array[unitPtr->race_id-1][unitPtr->nation_recno-1] = newValue;
	}
}
//------- End of function Town::update_target_resistance ------//


//------ Begin of function Town::camp_influence ------//
//
// Return a specific military camp's influence on this town.
//
// The influence is calculated based on :
// - the leadership of the commander
// - whether the leader is racially homogenous to this town.
// - whether the king is racially homogenous to this town.
// - the reputation of the nation
//
// <int> unitRecno - unit recno of the commander in the military camp.
//
// return: <int> influence index (0-100)
//
int Town::camp_influence(int unitRecno)
{
	Unit*   unitPtr = unit_array[unitRecno];
	Nation* nationPtr = nation_array[unitPtr->nation_recno];   // nation of the unit

	int thisInfluence = unitPtr->skill.get_skill(SKILL_LEADING)*2/3;		// 66% of the leadership

	if( race_res.is_same_race(nationPtr->race_id, unitPtr->race_id) )
		thisInfluence += thisInfluence/3;		// 33% bonus if the king's race is also the same as the general

	thisInfluence += (int) nationPtr->reputation/2;;

	thisInfluence = MIN(100, thisInfluence);

	return thisInfluence;
}
//------- End of function Town::camp_influence ------//


//------ Begin of function Town::update_resistance -----//
//
// Update loyalty and resistance towards the target.
//
void Town::update_resistance()
{
	//------------- update resistance ----------------//

	int	i, j, maxNation = nation_array.size();
	char	zeroResistance[MAX_NATION];
	float targetResistance;

	memset( zeroResistance, 1, MAX_NATION );

	for(i=0; i<MAX_RACE; i++)
	{
		if( race_pop_array[i]==0 )
		{
			for(j=0; j<maxNation; j++)							// reset resistance for non-existing races
				race_resistance_array[i][j] = (float) 0;

			continue;
		}

		for(j=0; j<maxNation; j++)
		{
			if( nation_array.is_deleted(j+1) )
				continue;

			if( race_target_resistance_array[i][j] >= 0 )
			{
				targetResistance = (float) race_target_resistance_array[i][j];

				if(race_resistance_array[i][j] > targetResistance)		// only decrease, no increase for resistance
				{
					float decValue = (race_resistance_array[i][j]-targetResistance) / 30;

					race_resistance_array[i][j] -= MAX(1, decValue);

					if(race_resistance_array[i][j] < targetResistance) // avoid resistance oscillate between taregtLoyalty-1 and taregtLoyalty+1
						race_resistance_array[i][j] = (float)targetResistance;
				}
			}

			if( race_resistance_array[i][j] >= (float) 1 )		// also values between consider 0 and 1 as zero as they are displayed as 0 in the interface
				zeroResistance[j] = 0;
		}
	}

	//--- if the town is zero resistance towards any nation, convert to that nation ---//

	for(j=0; j<maxNation; j++)
	{
		if( nation_array.is_deleted(j+1) )
			continue;

		if( zeroResistance[j] )
		{
			surrender(j+1);
			break;
		}
	}
}
//------- End of function Town::update_resistance ------//


//---------- Begin of function Town::collect_yearly_tax --------//
//
// Collect yearly tax from villagers - this does not decrease their
// loyalty.
//
void Town::collect_yearly_tax()
{
	nation_array[nation_recno]->add_income(INCOME_TAX, (float)population * TAX_PER_PERSON );
}
//----------- End of function Town::collect_yearly_tax ---------//


//---------- Begin of function Town::collect_tax --------//
//
// Collect tax from this town.
//
void Town::collect_tax(char remoteAction)
{
	if( !has_linked_own_camp )
		return;

	//------------------------------------------//

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <town recno> <race id>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_COLLECT_TAX, sizeof(short));
		shortPtr[0] = town_recno;
		return;
	}

	//----- calculate the loyalty decrease amount ------//
	//
	// If you reward too frequently, the negative effect
	// on loyalty will get larger.
	//
	//--------------------------------------------------//

	int loyaltyDecrease = COLLECT_TAX_LOYALTY_DECREASE + accumulated_collect_tax_penalty/5;

	loyaltyDecrease = MIN(loyaltyDecrease, COLLECT_TAX_LOYALTY_DECREASE+10);

	accumulated_collect_tax_penalty += 10;

	//------ decrease the loyalty of the town people ------//

	// ##### patch begin Gilbert 5/8 ######//
//	for( int i=0 ; i<MAX_RACE ; i++ )
//		change_loyalty( i+1, (float) -loyaltyDecrease );
	//----------- increase cash ------------//
//	nation_array[nation_recno]->add_income(INCOME_TAX, (float)population * TAX_PER_PERSON );

	// ------ cash increase depend on loyalty drop --------//
	float taxCollected = 0.0f;
	for( int i = 0; i < MAX_RACE; i++ )
	{
		float beforeLoyalty = race_loyalty_array[i];
		change_loyalty( i+1, (float) -loyaltyDecrease );
		taxCollected += (beforeLoyalty - race_loyalty_array[i]) * race_pop_array[i] * TAX_PER_PERSON / loyaltyDecrease;
	}

	//----------- increase cash ------------//

	nation_array[nation_recno]->add_income(INCOME_TAX, taxCollected );

	// ##### patch end Gilbert 5/8 ######//

	//------------ think rebel -------------//

	think_rebel();
}
//----------- End of function Town::collect_tax ---------//


//---------- Begin of function Town::reward --------//
//
// Reward cash to the peasants.
//
void Town::reward(char remoteAction)
{
	if( !has_linked_own_camp )
		return;

	//------------------------------------------//

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <town recno> <race id>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_REWARD, sizeof(short));
		shortPtr[0] = town_recno;
		return;
	}

	//----- calculate the loyalty increase amount ------//
	//
	// If you reward too frequently, the effect of the
	// granting will be diminished.
	//
	//--------------------------------------------------//

	int loyaltyIncrease = TOWN_REWARD_LOYALTY_INCREASE - accumulated_reward_penalty/5;

	loyaltyIncrease = MAX(3, loyaltyIncrease);

	accumulated_reward_penalty += 10;

	//------ increase the loyalty of the town people ------//

	for( int i=0 ; i<MAX_RACE ; i++ )
		change_loyalty( i+1, (float) loyaltyIncrease );

	//----------- decrease cash ------------//

	nation_array[nation_recno]->add_expense(EXPENSE_GRANT_OWN_TOWN, (float)population * TOWN_REWARD_PER_PERSON );
}
//----------- End of function Town::reward ---------//


//------- Begin of function Town::think_surrender ---------//
//
// Think about surrendering to a nation with camps linked to
// this town.
//
int Town::think_surrender()
{
	if( !nation_recno )			// if it's an independent town
		return 0;

	//--- only surrender when there is no own camps, but has enemy camps linked to this town ---//

	if( has_linked_own_camp || !has_linked_enemy_camp )
		return 0;

	//--- surrender if 2/3 of the population think about surrender ---//

	int i, discontentedCount=0;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] <= race_spy_count_array[i] )	// spies do not rebel together with the rebellion
			continue;

		if( race_loyalty_array[i] <= SURRENDER_LOYALTY )
			discontentedCount += race_pop_array[i];
	}

	if( discontentedCount < population * 2 / 3 )
		return 0;

	//-------- think about surrender to which nation ------//

	int   curRating, bestRating=average_loyalty(), bestNationRecno=0;
	Firm* firmPtr;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[linked_firm_array[i]];

		//---- if this is an enemy camp ----//

		if( firmPtr->firm_id == FIRM_CAMP &&
			 firmPtr->nation_recno != nation_recno &&
			 firmPtr->nation_recno &&
			 firmPtr->overseer_recno )
		{
			curRating = camp_influence(firmPtr->overseer_recno);		// see camp_influence() for details on how the rating is calculated

			if( curRating > bestRating )
			{
				bestRating = curRating;
				bestNationRecno = firmPtr->nation_recno;
			}
		}
	}

	//------------------------------------//

	if( bestNationRecno )
	{
		surrender( bestNationRecno );
		return 1;
	}
	else
		return 0;
}
//------- End of function Town::think_surrender ---------//


//------- Begin of function Town::surrender ---------//
//
// This town surrenders to another nation.
//
// <int> toNationRecno - the recno of the nation this town surrenders to.
//
void Town::surrender(int toNationRecno)
{
	err_when( !toNationRecno );

	//--- if this is a rebel town and the mobile rebel count is > 0, don't surrender (this function can be called by update_resistance() when resistance drops to zero ---//

	if( rebel_recno )
	{
		Rebel* rebelPtr = rebel_array[rebel_recno];

		if( rebelPtr->mobile_rebel_count > 0 )
			return;
	}

	//----------------------------------------//

	if( nation_recno  == nation_array.player_recno ||
		 toNationRecno == nation_array.player_recno )
	{
		news_array.town_surrendered(town_recno, toNationRecno);
		// ##### begin Gilbert 9/10 ######//
		// sound effect
		if( toNationRecno == nation_array.player_recno )
		{
			se_ctrl.immediate_sound("GET_TOWN");
		}
		// ##### end Gilbert 9/10 ######//
	}

	set_nation( toNationRecno );
}
//------- End of function Town::surrender ---------//


//------- Begin of function Town::think_rebel ---------//
//
void Town::think_rebel()
{
	if( !nation_recno )
		return;

	#define REBEL_INTERVAL_MONTH		3		// don't rebel twice in less than 3 months

	if( info.game_date < last_rebel_date + REBEL_INTERVAL_MONTH*30 )
		return;

	if( town_defender_count > 0 || info.game_date < last_being_attacked_date + 10 )		// don't rebel within ten days after being attacked by a hostile unit
		return;

	//--- rebel if 2/3 of the population becomes discontented ---//

	int i, discontentedCount=0, rebelLeaderRaceId=0, largestRebelRace=0, trainRaceId=0;
	int restrictRebelCount[MAX_RACE];

	if( train_unit_recno )
		trainRaceId = unit_array[train_unit_recno]->race_id;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		restrictRebelCount[i] = race_spy_count_array[i];  // spies do not rebel together with the rebellion

		if( race_pop_array[i]>0 && race_loyalty_array[i] <= REBEL_LOYALTY )
		{
			discontentedCount += race_pop_array[i];

			// count firm spies that reside in this town
			for( int j=0 ; j<linked_firm_count ; j++ )
			{
				Firm* firmPtr = firm_array[linked_firm_array[j]];
				for( int k=0 ; k<firmPtr->worker_count ; k++ )
				{
					Worker* workerPtr = firmPtr->worker_array+k;
					if( workerPtr->spy_recno && workerPtr->town_recno == town_recno )
						restrictRebelCount[i]++;
				}
			}

			if( trainRaceId==i+1 )
				restrictRebelCount[i]++; // unit under training cannot rebel

			if( race_pop_array[i] <= restrictRebelCount[i] )
				continue; // no one can lead from this group

			if( race_pop_array[i] > largestRebelRace )
			{
				largestRebelRace  = race_pop_array[i];
				rebelLeaderRaceId = i+1;
			}
		}
	}

	if( !rebelLeaderRaceId ) // no discontention or no one can lead
		return;

	if( population == 1 )			// if population is 1 only, handle otherwise
	{
	}
	else
	{
		if( discontentedCount < population * 2 / 3 )
			return;
	}

	//----- if there was just one unit in the town and he rebels ----//

	int oneRebelOnly=0;

	if( population==1 )
	{
		news_array.town_rebel(town_recno, 1);
		oneRebelOnly = 1;
	}

	//----- create the rebel leader and the rebel group ------//

	int rebelCount=1;
	int leaderUnitRecno = create_rebel_unit(rebelLeaderRaceId, 1);		// 1-the unit is the rebel leader

	if( !leaderUnitRecno )
		return;

	uint32_t curGroupId = unit_array.cur_group_id++;
	Unit *unitPtr = unit_array[leaderUnitRecno];
	unitPtr->unit_group_id = curGroupId;

	if( oneRebelOnly )		// if there was just one unit in the town and he rebels
	{
		rebel_array.create_rebel(leaderUnitRecno, nation_recno);
		return;
	}

	int rebelRecno = rebel_array.create_rebel(leaderUnitRecno, nation_recno, REBEL_ATTACK_TOWN, town_recno);		// create a rebel group

	//------- create other rebel units in the rebel group -----//

	int j, unitRecno, raceRebelCount;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] <= restrictRebelCount[i] || race_loyalty_array[i] > REBEL_LOYALTY )
			continue;

		if( population==1 )		// if only one peasant left, break, so not all peasants will rebel 
			break;

		raceRebelCount = (int) (race_pop_array[i]-restrictRebelCount[i]) * (30+misc.random(30)) / 100;		// 30% - 60% of the unit will rebel.
		err_when(raceRebelCount+1 > MAX_TOWN_POPULATION); // plus 1 for the leader, cannot excess MAX_TOWN_POPULATION, consider the case these units settle immediately

		for( j=0 ; j<raceRebelCount ; j++ )		// no. of rebel units of this race
		{
			unitRecno = create_rebel_unit(i+1, 0);

			if( !unitRecno )		// 0-the unit is not the rebel leader
				break;

			unitPtr = unit_array[unitRecno];
			unitPtr->unit_group_id = curGroupId;
			unitPtr->leader_unit_recno = leaderUnitRecno;

			rebel_array[rebelRecno]->join(unitRecno);

			rebelCount++;
		}

		//--- when disloyal units left, the average loyalty increases ---//

		change_loyalty( i+1, (float) 50 * j / race_pop_array[i] );
	}

	//---------- add news -------------//

	last_rebel_date = info.game_date;

	news_array.town_rebel(town_recno, rebelCount);		// add the news first as after callijng ai_spy_town_rebel, the town may disappear as all peasants are gone

	//--- tell the AI spies in the town that a rebellion is happening ---//

	spy_array.ai_spy_town_rebel(town_recno);
}
//------- End of function Town::think_rebel ---------//


//-------- Begin of function Town::create_rebel_unit --------//
//
// Create a rebel unit from a town.
//
// <int> raceId   - race id. of the unit
// <int> isLeader - whether the unit is a rebel leader
//
// return: <int> recno of the unit created.
//
int Town::create_rebel_unit(int raceId, int isLeader)
{
/*	//--- do not mobilize spies as rebels ----//

	//---------------------------------------//
	//
	// If there are spies in this town, first mobilize
	// the spies whose actions are "Sow Dissent".
	//
	//---------------------------------------//

	int idleSpyCount=0;

	if( race_spy_count_array[raceId-1] > 0 )
	{
		Spy* spyPtr;

		for( int i=spy_array.size() ; i>0 ; i-- )
		{
			if( spy_array.is_deleted(i) )
				continue;

			spyPtr = spy_array[i];

			if( spyPtr->spy_place==SPY_TOWN &&
				 spyPtr->spy_place_para==town_recno &&
				 spyPtr->race_id==raceId )
			{
				if( spyPtr->action_mode == SPY_SOW_DISSENT )
				{
					int unitRecno = spyPtr->mobilize_town_spy();

					if( isLeader )
						unit_array[unitRecno]->set_rank(RANK_GENERAL);

					return unitRecno;
				}

				idleSpyCount++;
			}
		}
	}
	//---- if the remaining population are all sleep spy, no new rebels ----//

	if( race_pop_array[raceId-1] == idleSpyCount )
		return 0;

	err_when( race_pop_array[raceId-1] < idleSpyCount );
*/

	//---- if no jobless people, make workers and overseers jobless ----//

	if( recruitable_race_pop(raceId, 0)==0 )	// 0-don't recruit spies as the above code should have handle spies already
	{
		if( !unjob_town_people(raceId, 0, 0) )		// 0-don't unjob spies, 0-don't unjob overseer
			return 0;

		if( recruitable_race_pop(raceId,0)==0 )	// if the unjob unit is a spy too, then don't rebel
			return 0;
	}

	//----look for an empty locatino for the unit to stand ----//
	//--- scan for the 5 rows right below the building ---//

	int            unitId = race_res[raceId]->basic_unit_id;
	SpriteInfo*    spriteInfo = sprite_res[unit_res[unitId]->sprite_id];
	int            xLoc=loc_x1, yLoc=loc_y1;     // xLoc & yLoc are used for returning results

	if( !world.locate_space( &xLoc, &yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height ) )
		return 0;

	//---------- add the unit now -----------//

	int rankId;

	if( isLeader )
		rankId = RANK_GENERAL;
	else
		rankId = RANK_SOLDIER;

	int unitRecno = unit_array.add_unit( unitId, 0, rankId, 0, xLoc, yLoc );

	dec_pop(raceId, 0);		// decrease the town's population

	//------- set the unit's parameters --------//

	Unit* unitPtr = unit_array[unitRecno];

	if( isLeader )
	{
		int combatLevel 	  = 10 + population*2 + misc.random(10);		// the higher the population is, the higher the combat level will be
		int leadershipLevel = 10 + population   + misc.random(10);		// the higher the population is, the higher the combat level will be

		unitPtr->set_combat_level( MIN(combatLevel, 100) );

		unitPtr->skill.skill_id 	= SKILL_LEADING;
		unitPtr->skill.skill_level = MIN(leadershipLevel, 100);
	}
	else
	{
		unitPtr->set_combat_level(CITIZEN_COMBAT_LEVEL); 	// combat: 10
	}

	return unitRecno;
}
//--------- End of function Town::create_rebel_unit ---------//


//--------- Begin of function Town::assign_unit --------//
//
void Town::assign_unit(int unitRecno)
{
	//-----------------------------------------//

	Unit* unitPtr = unit_array[unitRecno];

	if( population >= MAX_TOWN_POPULATION || unitPtr->rank_id == RANK_KING )
	{
		unitPtr->stop2();
		//----------------------------------------------------------------------//
		// codes for handle_blocked_move 
		// set unit_group_id to a different value s.t. the members in this group
		// will not blocked by this unit.
		//----------------------------------------------------------------------//
		unitPtr->unit_group_id = unit_array.cur_group_id++;
		return;
	}

	//------ if the unit is a general, demote it first -------//

	if( unitPtr->rank_id == RANK_GENERAL )
		unitPtr->set_rank(RANK_SOLDIER);

	//-------- increase population -------//

	inc_pop(unitPtr->race_id, 0, unitPtr->loyalty);

	//---- free the unit's name from the name database ----//

	race_res[unitPtr->race_id]->free_name_id( unitPtr->name_id );

	//----- if it's a town defending unit -----//

	if( unitPtr->unit_mode == UNIT_MODE_DEFEND_TOWN )
	{
		#define RESISTANCE_INCREASE  2   // if the defender defeat the attackers and return the town with victory, the resistance will further increase

		//---------------------------------------------//
		//
		// If this unit is a defender of the town, add back the
		// loyalty which was deducted from the defender left the
		// town.
		//
		//---------------------------------------------//

		if( unitPtr->nation_recno == nation_recno &&
			 unitPtr->unit_mode_para == town_recno )
		{
			//-- if the unit is a town defender, skill.skill_level is temporary used for storing the loyalty that will be added back to the town if the defender returns to the town

			int loyaltyInc = unitPtr->skill.skill_level;

			if( nation_recno )			// set the loyalty later for nation_recno > 0
			{
				change_loyalty( unitPtr->race_id, (float) loyaltyInc );
			}
			else
			{
				for( int i=0 ; i<MAX_NATION ; i++ )		// set the resistance
				{
					float newResistance = race_resistance_array[unitPtr->race_id-1][i]
												 + loyaltyInc + RESISTANCE_INCREASE;

					race_resistance_array[unitPtr->race_id-1][i]	= MIN(newResistance, 100);
				}
			}
		}
	}

	//------ if the unit is a spy -------//

	if( unitPtr->spy_recno > 0 )
	{
		spy_array[unitPtr->spy_recno]->set_place(SPY_TOWN, town_recno);
		unitPtr->spy_recno = 0;		// reset it so Unit::deinit() won't delete the spy
	}

	//----- if this is an independent town -----//

	if( nation_recno==0 )		// update the town people's combat level with this unit's combat level
	{
		town_combat_level = ( (int)town_combat_level*(population-1) + (int)unitPtr->skill.combat_level ) / population;
	}

	//--------- delete the unit --------//

	unit_array.disappear_in_town(unitRecno, town_recno);		// the unit disappear from the map because it has moved into a town
}
//---------- End of function Town::assign_unit --------//


//------- Begin of function Town::think_migrate ---------//
//
// Let the town people (only those jobless) think if they want
// to migrate or not.
//
// People who have jobs will think in Firm::think_worker_migrate()
//
// Migration Possibilities:
//
// Independent town people migrate to -> nation town
// Nation town people migrate to -> nation town
//
// But nation town people will NOT migrate to independent towns.
//
void Town::think_migrate()
{
	#define MAX_MIGRATE_PER_DAY	 4			// don't migrate more than 4 units per day

	if( jobless_population==0 )
		return;

	int   i, j, raceId, migratedCount, townDistance;
	int   saveTownRecno = town_recno;
	int	saveTownNationRecno = nation_recno;
	Town* townPtr;

	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( !townPtr->nation_recno )
			continue;

		if( townPtr->town_recno == town_recno )
			continue;

		if( townPtr->population>=MAX_TOWN_POPULATION )
			continue;

		townDistance = misc.points_distance(center_x, center_y, townPtr->center_x, townPtr->center_y);

#ifndef ENABLE_LONG_DISTANCE_MIGRATION
		if( townDistance > EFFECTIVE_TOWN_TOWN_DISTANCE )
			continue;
#endif

		//---- scan all jobless population, see if any of them want to migrate ----//

		raceId = random_race();

		for( j=0 ; j<MAX_RACE ; j++ )
		{
			if( ++raceId > MAX_RACE )
				raceId = 1;

			err_when( race_spy_count_array[raceId-1] < 0 );

			if( recruitable_race_pop(raceId, 0)==0 )		// only if there are peasants who are jobless and are not spies
				continue;

			//--- migrate a number of people of the same race at the same time ---//

			migratedCount=0;

			int loopCount=0;

			while( think_migrate_one(townPtr, raceId, townDistance) )
			{
				err_when( ++loopCount > 100 );

				migratedCount++;

				if( townDistance > EFFECTIVE_TOWN_TOWN_DISTANCE )		// don't migrate more than one unit at a time for migrating to non-linked towns
					break;

				if( migratedCount >= MAX_MIGRATE_PER_DAY || misc.random(4)==0 ) // allow a random and low max number to migrate when this happens
					break;
			}

			//------------- add news --------------//

			if( migratedCount > 0 )
			{
				if( saveTownNationRecno==nation_array.player_recno ||
					 townPtr->nation_recno==nation_array.player_recno )
				{
					news_array.migrate(saveTownRecno, townPtr->town_recno, raceId, migratedCount);
				}

				return;
			}
		}
	}
}
//-------- End of function Town::think_migrate -----------//


//------- Begin of function Town::think_migrate_one ---------//
//
// Think about migrating one person of the given race to the
// given town.
//
//------------------------------------------------//
//
// Calculate the attractive factor, it is based on:
//
// - the reputation of the target nation (+0 to 100)
// - the racial harmony of the race in the target town (+0 to 100)
// - distance between the current town and the target town (-0 to 100)
//
// Attractiveness level range: 0 to 200
//
//------------------------------------------------//
//
int Town::think_migrate_one(Town* targetTown, int raceId, int townDistance)
{
	#define MIN_MIGRATE_ATTRACT_LEVEL	30

	//-- only if there are peasants who are jobless and are not spies --//

	if( recruitable_race_pop(raceId,0)==0 )		//0-don't recruit spies
		return 0;

	//---- if the target town's population has already reached its MAX ----//

	if( targetTown->population>=MAX_TOWN_POPULATION )
		return 0 ;

	//-- do not migrate if the target town's population of that race is less than half of the population of the current town --//

	if( targetTown->race_pop_array[raceId-1] < race_pop_array[raceId-1]/2 )
		return 0;

	//-- do not migrate if the target town might not be a place this peasant will stay --//

	if( targetTown->race_loyalty_array[raceId-1] < 40 )
		return 0;

	//--- calculate the attractiveness rating of the current town ---//

	int curAttractLevel = race_harmony(raceId);

#ifdef ENABLE_LONG_DISTANCE_MIGRATION
	//--- if the target town is not linked to the current town, reduce attractiveness ---//

	if( townDistance > EFFECTIVE_TOWN_TOWN_DISTANCE )
	{
		curAttractLevel -= 20 + townDistance/2;		// 20 to 70 negative
	}
#endif

	//------- loyalty/resistance affecting the attractivness ------//

	if( nation_recno )
	{
		curAttractLevel += (int) nation_array[nation_recno]->reputation +
								 (int) (race_loyalty_array[raceId-1] - 40);  // loyalty > 40 is considered as positive force, < 40 is considered as negative force
	}
	else
	{
		if( targetTown->nation_recno )
			curAttractLevel += (int) race_resistance_array[raceId-1][targetTown->nation_recno-1];
	}

	//--- calculate the attractiveness rating of the target town ---//

	int targetAttractLevel = targetTown->race_harmony(raceId);

	if( targetTown->nation_recno )
		targetAttractLevel += (int) nation_array[targetTown->nation_recno]->reputation;

	if( targetAttractLevel < MIN_MIGRATE_ATTRACT_LEVEL )
		return 0;

	//--------- compare the attractiveness ratings ---------//

	if( targetAttractLevel - curAttractLevel > MIN_MIGRATE_ATTRACT_LEVEL/2 )
	{
#ifdef ENABLE_LONG_DISTANCE_MIGRATION
		//--- if this is non-linked town, there are 50% chance that the migrating units will get lost on their way and never reach the destination ---//

		if( townDistance > EFFECTIVE_TOWN_TOWN_DISTANCE )
		{
			if( misc.random(2)==0 )
			{
				dec_pop(raceId, 0);
				return 1;
			}
		}
#endif

		//---------- migrate now ----------//

		int newLoyalty = MAX( targetAttractLevel/2, 40 );

		migrate(raceId, targetTown->town_recno, newLoyalty);
		return 1;
	}

	return 0;
}
//-------- End of function Town::think_migrate_one -----------//


//------- Begin of function Town::migrate ---------//
//
// People migrate from one town to another.
//
// <int> raceId        - id. of the race that migrates
// <int> destTownRecno - recno of the destination town.
// <int> newLoyalty    - loyalty of the unit in the target town.
//
// return: <int> 1 - migrated successfully
//               0 - no building space in the target town for that race
//
void Town::migrate(int raceId, int destTownRecno, int newLoyalty)
{
	Town* destTown = town_array[destTownRecno];

	if(destTown->population>=MAX_TOWN_POPULATION)
		return;

	//------- decrease the population of this town ------//

	dec_pop(raceId, 0);

	//--------- increase the population of the target town ------//

	destTown->inc_pop(raceId, 0, newLoyalty);
}
//-------- End of function Town::migrate -----------//


//------- Begin of function Town::migrate_to ---------//

int Town::migrate_to(int destTownRecno, char remoteAction, int raceId, int count)
{
	if (count <= 0 || count > MAX_TOWN_POPULATION)
	{
		err_here();
		return 0;
	}

	if( !raceId )
	{
		raceId = browse_selected_race_id();

		if( !raceId )
			return 0;
	}

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <town recno> <dest town recno> <race id> <count>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_MIGRATE, 4*sizeof(short));
		shortPtr[0] = town_recno;
		shortPtr[1] = destTownRecno;
		shortPtr[2] = raceId;
		shortPtr[3] = count;
		return 0;
	}

	bool continueMigrate = true;
	int migrated = 0;
	while( continueMigrate && migrated < count )
	{
		continueMigrate = can_migrate(destTownRecno, 1, raceId); // 1- migrate now, 1-allow migrate spy
		if (continueMigrate) ++migrated;
	}

	return migrated;
}
//-------- End of function Town::migrate_to -----------//


//------- Begin of function Town::can_migrate ---------//
//
// Check if it is okay for migrating one person from
// the current town to the given town.
//
// <int> destTownRecno - the recno of the town the people
//								 will migrate to.
//
// [int] migrateNow 	  - migrate now or not if the result is true
//								 (default: false)
//
// [int] raceId        - race to check migrate
//                       (default: 0, to find from the selected_race_id() )
//
// return: <int> true - migration allowed
//					  false - migration not allowed
//
bool Town::can_migrate(int destTownRecno, bool migrateNow, int raceId)
{
	if(!raceId)
	{
		raceId = browse_selected_race_id();

		if( !raceId )
			return false;
	}

	Town* destTown = town_array[destTownRecno];

	if( destTown->population>=MAX_TOWN_POPULATION )
		return false;

	//---- if there are still jobless units ----//

	if( recruitable_race_pop(raceId, 1) > 0 )		// 1-allow migrate spy 
	{
		if( migrateNow )
			move_pop(destTown, raceId, 0);		// 0-doesn't have job 

		return true;
	}

	//--- if there is no jobless units left -----//

	int 	  i, j;
	Firm*   firmPtr;
	Worker* workerPtr;

	if( race_pop_array[raceId-1] > 0 )
	{
		//---- scan for firms that are linked to this town ----//

		for( i=linked_firm_count-1 ; i>=0 ; i-- )
		{
			firmPtr = firm_array[linked_firm_array[i]];

			//---- only for firms whose workers live in towns ----//

			if( !firm_res[firmPtr->firm_id]->live_in_town )
				continue;

			//---- if the target town is within the effective range of this firm ----//

			if( misc.points_distance( destTown->center_x, destTown->center_y,
				 firmPtr->center_x, firmPtr->center_y ) > EFFECTIVE_FIRM_TOWN_DISTANCE )
			{
				continue;
			}

			//------- scan for workers -----------//

			workerPtr=firmPtr->worker_array;

			for( j=firmPtr->worker_count-1 ; j>=0 ; j--, workerPtr++ )
			{
				//--- if the worker lives in this town ----//

				if( workerPtr->race_id == raceId &&
					 workerPtr->town_recno == town_recno )
				{
					if( migrateNow )
					{
						if( firm_res[firmPtr->firm_id]->live_in_town )
							workerPtr->town_recno = destTownRecno;
						else
							workerPtr->town_recno = 0;

						move_pop(destTown, raceId, 1);		// 1-has job
					}

					return true;
				}
			}
		}
	}

	return false;
}
//-------- End of function Town::can_migrate -----------//


//------- Begin of function Town::move_pop ---------//
//
// This function is called by Town::can_migrate() only to move a unit
// from one town to another.
//
void Town::move_pop(Town* destTown, int raceId, int hasJob)
{
	//--- if the only pop of this race in the source town are spies ---//

	if( !hasJob )		// only for peasant, for job unit, spy_place==SPY_FIRM and it isn't related to race_spy_count_array[] 
	{
		err_when( race_spy_count_array[raceId-1] > jobless_race_pop_array[raceId-1] );

		if( race_spy_count_array[raceId-1] == jobless_race_pop_array[raceId-1] )
		{
			int spySeqId = misc.random( race_spy_count_array[raceId-1] ) + 1;		// randomly pick one of the spies

			int spyRecno = spy_array.find_town_spy(town_recno, raceId, spySeqId);

			err_when( !spyRecno );

			spy_array[spyRecno]->spy_place_para = destTown->town_recno;		// set the place_para of the spy

			race_spy_count_array[raceId-1]--;
			destTown->race_spy_count_array[raceId-1]++;
		}
	}

	//------------------------------------------//

	err_when( destTown->population>MAX_TOWN_POPULATION );

	destTown->inc_pop( raceId, hasJob, (int) race_loyalty_array[raceId-1] );

	dec_pop( raceId, hasJob );		// the unit doesn't have a job - this must be called finally as dec_pop() will have the whole town deleted if there is only one pop left

	err_when( race_spy_count_array[raceId-1] > jobless_race_pop_array[raceId-1] );
	err_when( destTown->race_spy_count_array[raceId-1] > destTown->jobless_race_pop_array[raceId-1] );
}
//-------- End of function Town::move_pop -----------//


//-------- Begin of function Town::mobilize_town_people --------//
//
// Call out a citizen of the town population and make it
// as an unit.
//
// <int> raceId - race id. of the unit
// <int> decPop - whether decrease the population or not,
//						if 0, only a unit will be created and no population will
//						be reduced.
// <int> mobileSpyFlag - whether spies will be mobilized
//
// return: <int> recno of the unit created.
//
int Town::mobilize_town_people(int raceId, int decPop, int mobileSpyFlag)
{
	//---- if no jobless people, make workers and overseers jobless ----//

	if( recruitable_race_pop(raceId, mobileSpyFlag)==0 )
	{
		if( !unjob_town_people(raceId, mobileSpyFlag, 0) )		// 0-don't unjob overseer
			return 0;

		err_when( recruitable_race_pop(raceId, mobileSpyFlag)==0 );
	}

	//----look for an empty locatino for the unit to stand ----//
   //--- scan for the 5 rows right below the building ---//

   int            unitId = race_res[raceId]->basic_unit_id;
   SpriteInfo*    spriteInfo = sprite_res[unit_res[unitId]->sprite_id];
   int            xLoc=loc_x1, yLoc=loc_y1;     // xLoc & yLoc are used for returning results

   if( !world.locate_space( &xLoc, &yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height ) )
      return 0;

	//---------- add the unit now -----------//

	int unitRecno = unit_array.add_unit( unitId,
						 nation_recno, RANK_SOLDIER, (int) race_loyalty_array[raceId-1], xLoc, yLoc );

   //------- set the unit's parameters --------//

	Unit* unitPtr = unit_array[unitRecno];

	unitPtr->set_combat_level(CITIZEN_COMBAT_LEVEL);

	//-------- decrease the town's population ------//

	if( decPop )
		dec_pop(raceId, 0);

	return unitRecno;
}
//--------- End of function Town::mobilize_town_people ---------//


//--------- Begin of function Town::being_attacked -------//
//
// This function is called by Unit::hit_town()
//
// <int>   attackerUnitRecno - recno of the unit attacking this town.
// <float> attackDamage      - damage caused by the attack
//
void Town::being_attacked(int attackerUnitRecno, float attackDamage)
{
	if( rebel_recno )		// if this town is controlled by a rebel group
		rebel_array[rebel_recno]->town_being_attacked(attackerUnitRecno);

	if( population==0 )
		return;

	defend_target_recno = attackerUnitRecno; // store the target recno

	Unit* attackerUnit = unit_array[attackerUnitRecno];

	if( attackerUnit->nation_recno == nation_recno )		// this can happen when the unit has just changed nation 
		return;

	int attackerNationRecno = attackerUnit->nation_recno;

	last_being_attacked_date = info.game_date;

	//--------- store attacker nation recno -----------//

	err_when(attackerNationRecno<0 || attackerNationRecno>MAX_NATION);
	set_hostile_nation(attackerNationRecno);

	//----------- call out defender -----------//

	// only call out defender when the attacking unit is within the effective defending distance

	if( misc.points_distance( attackerUnit->cur_x_loc(), attackerUnit->cur_y_loc(),
		 center_x, center_y ) <= EFFECTIVE_DEFEND_TOWN_DISTANCE )
	{
		int loopCount=0;

		while(1)
		{
			if( !mobilize_defender(attackerNationRecno) )
				break;

			err_when( loopCount++ > 1000 );
		}
	}

	auto_defense(attackerUnitRecno);

	//----- pick a race to be attacked by the attacker randomly -----//

	int raceId = pick_random_race(1, 1);		// 1-pick has job people, 1-pick spies

	err_when( !raceId );

	//-------- town people get killed ---------//

	received_hit_count += attackDamage;

	if( received_hit_count >= RECEIVED_HIT_PER_KILL )
	{
		received_hit_count = (float) 0;

		int townRecno = town_recno;

		kill_town_people(raceId, attackerNationRecno);		// kill a town people

		if( town_array.is_deleted(townRecno) )		// the town may have been deleted when all pop are killed
			return;
	}

	//---- decrease resistance if this is an independent town ----//

	if( town_defender_count==0 )
	{
		//--- Resistance/loyalty of the town people decrease if the attacking continues ---//
		//
		// Resistance/Loyalty decreases faster:
		//
		// -when there are few people in the town
		// -when there is no defender
		//
		//---------------------------------------//

		float loyaltyDec;

		if( nation_recno )		// if the town belongs to a nation
		{
			//---- decrease loyalty of all races in the town ----//

			for( raceId=1 ; raceId<=MAX_RACE ; raceId++ )
			{
				if( race_pop_array[raceId-1] == 0 )
					continue;

				if( has_linked_own_camp )		// if it is linked to one of its camp, the loyalty will decrease slower
					loyaltyDec = (float) 5 / race_pop_array[raceId-1];
				else
					loyaltyDec = (float) 10 / race_pop_array[raceId-1];

				loyaltyDec = MIN( loyaltyDec, (float) 1 );

				change_loyalty( raceId, -loyaltyDec * attackDamage / (20/ATTACK_SLOW_DOWN) );
			}

			//--- if the resistance of all the races are zero, think_change_nation() ---//

			int i;
			for( i=0 ; i<MAX_RACE ; i++ )
			{
				if( race_loyalty_array[i] >= (float) 1 )		// values between 0 and 1 are considered as 0
					break;
			}

			if( i==MAX_RACE )                  		// if resistance of all races drop to zero
				think_surrender();
		}
		else						// if the town is an independent town
		{
			if( !attackerNationRecno )		// if independent units do not attack independent towns
				return;

			//---- decrease resistance of all races in the town ----//

			for( raceId=1 ; raceId<=MAX_RACE ; raceId++ )
			{
				if( race_pop_array[raceId-1] == 0 )
					continue;

				loyaltyDec = (float) 10 / race_pop_array[raceId-1];		// decrease faster for independent towns than towns belonging to nations
				loyaltyDec = MIN( loyaltyDec, (float) 1 );

				race_resistance_array[raceId-1][attackerNationRecno-1] -= loyaltyDec * attackDamage / (20/ATTACK_SLOW_DOWN);

				if( race_resistance_array[raceId-1][attackerNationRecno-1] < 0 )
					race_resistance_array[raceId-1][attackerNationRecno-1] = (float) 0;
			}

			//--- if the resistance of all the races are zero, think_change_nation() ---//

			int i;
			for( i=0 ; i<MAX_RACE ; i++ )
			{
				if( race_resistance_array[i][attackerNationRecno-1] >= (float) 1 )
					break;
			}

			if( i==MAX_RACE )                  		// if resistance of all races drop to zero
				surrender(attackerNationRecno);
		}
	}

	//------ reinforce troops to defend against the attack ------//

	if( town_defender_count==0 && nation_recno )
	{
		if( attackerUnit->nation_recno != nation_recno )		// they may become the same when the town has been captured 
			nation_array[nation_recno]->ai_defend(attackerUnitRecno);
	}
}
//---------- End of function Town::being_attacked -----//


//--------- Begin of function Town::clear_defense_mode -------//
void Town::clear_defense_mode()
{
	//------------------------------------------------------------------//
	// change defense unit's to non-defense mode
	//------------------------------------------------------------------//
	Unit *unitPtr;
	for(int i=unit_array.size(); i>=1; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];
		if(!unitPtr)
			continue;

		err_when(unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE || unitPtr->hit_points<=0);
		if(unitPtr->in_defend_town_mode() && unitPtr->action_misc==ACTION_MISC_DEFEND_TOWN_RECNO &&
			unitPtr->action_misc_para==town_recno)
			unitPtr->clear_town_defend_mode(); // note: maybe, unitPtr->nation_recno != nation_recno
	}
}
//---------- End of function Town::clear_defense_mode -----//


//--------- Begin of function Town::mobilize_defender -------//
//
int Town::mobilize_defender(int attackerNationRecno)
{
	if( population==1 )		// do not call out defenders any more if there is only one person left in the town, otherwise the town will be gone.
		return 0;

	//------- pick a race to mobilize randomly --------//

	int randomPersonId = misc.random(population)+1;
	int popSum=0, raceId=0;

	int i;
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		popSum += race_pop_array[i];

		if( randomPersonId <= popSum )
		{
			raceId = i+1;
			break;
		}
	}

	if( raceId==0 )
		return 0;

	//---- check if the current loyalty allows additional defender ----//
	//
	// if the loyalty is high, then there will be more town defenders
	//
	//-----------------------------------------------------------------//

	float curLoyalty;

	if( nation_recno )
		curLoyalty = race_loyalty_array[raceId-1];
	else
	{
		if( !attackerNationRecno )		// if independent units do not attack independent towns
			return 0;

		curLoyalty = race_resistance_array[raceId-1][attackerNationRecno-1];
	}

	//--- only mobilize new defenders when there aren't too many existing ones ---//

	if( rebel_recno )		// if this town is controlled by rebels
	{
		if( curLoyalty	< town_defender_count*2 )		// rebel towns have more rebel units coming out to defend
			return 0;
	}
	else
	{
		if( curLoyalty	< town_defender_count*5 )		// if no more defenders are allowed for the current loyalty
			return 0;
	}

	//----- check if the loyalty/resistance is high enough -----//

	if( nation_recno )
	{
		if( curLoyalty < MIN_NATION_DEFEND_LOYALTY )
			return 0;
	}
	else
	{
		if( curLoyalty < MIN_INDEPENDENT_DEFEND_LOYALTY )
			return 0;
	}

	//------ check if there are peasants to defend ------//

	if( recruitable_race_pop(raceId, 0) == 0 ) 	// 0-don't recruit spies
		return 0;

	//---------- create a defender unit --------------------//

	//--------------------------------------------------------------//
	//									 loyalty of that race
	// decrease loyalty by: -------------------------------
	//								no. of town people of that race
	//--------------------------------------------------------------//

	float loyaltyDec = curLoyalty/race_pop_array[raceId-1];  // decrease in loyalty or resistance

	if( nation_recno )
	{
		change_loyalty( raceId, -loyaltyDec );
	}
	else
	{
		for( i=0 ; i<MAX_NATION ; i++ )
			race_resistance_array[raceId-1][i] -= loyaltyDec;
	}

	//------- mobilize jobless people if there are any -------//

	int unitRecno = mobilize_town_people(raceId, 1, 0);		// 1-dec pop, 0-don't mobilize spy town people

	Unit* unitPtr = unit_array[unitRecno];

	err_when( town_array.is_deleted(town_recno) );

	unitPtr->set_mode( UNIT_MODE_DEFEND_TOWN, town_recno );

	unitPtr->skill.skill_level = (char) loyaltyDec;	// if the unit is a town defender, this var is temporary used for storing the loyalty that will be added back to the town if the defender returns to the town

	int combatLevel = town_combat_level + misc.random(20) - 10;		// -10 to +10 random difference

	combatLevel = MIN(combatLevel, 100);
	combatLevel = MAX(combatLevel, 10);

	unitPtr->set_combat_level(combatLevel);

	//-----------------------------------------------------//
	// enable unit defend_town mode
	//-----------------------------------------------------//

	unitPtr->stop2();
	unitPtr->action_mode2 = ACTION_DEFEND_TOWN_DETECT_TARGET;
	unitPtr->action_para2 = UNIT_DEFEND_TOWN_DETECT_COUNT;

	unitPtr->action_misc 	  = ACTION_MISC_DEFEND_TOWN_RECNO;
	unitPtr->action_misc_para = town_recno;

	town_defender_count++;

	//------- if this town is controlled by rebels --------//

	if( rebel_recno )
		rebel_array[rebel_recno]->mobile_rebel_count++;		// increase the no. of mobile rebel units

	return unitRecno;
}
//---------- End of function Town::mobilize_defender -----//


//--------- Begin of function Town::reduce_defender_count -------//

void Town::reduce_defender_count()
{
	if( --town_defender_count==0 )
		independ_town_nation_relation = 0;

	//------- if this town is controlled by rebels --------//

	if( rebel_recno )
		rebel_array[rebel_recno]->mobile_rebel_count--;		// decrease the no. of mobile rebel units
}
//---------- End of function Town::reduce_defender_count -----//


//--------- Begin of function Town::kill_town_people -------//
//
void Town::kill_town_people(int raceId, int attackerNationRecno)
{
	if( !raceId )
		raceId = pick_random_race(1, 1);		// 1-pick has job unit, 1-pick spies 

	if( !raceId )
		return ;

	//---- jobless town people get killed first, if all jobless are killed, then kill workers ----//

	if( recruitable_race_pop(raceId,1)==0 )
	{
		if( !unjob_town_people(raceId, 1, 1) )				// 1-unjob spies, 1-unjob overseer if the only person left is a overseer
			return;

		err_when( recruitable_race_pop(raceId,1)==0 );
	}

	//------ the killed unit can be a spy -----//

	if( misc.random(recruitable_race_pop(raceId,1)) < race_spy_count_array[raceId-1] )
	{
		int spyRecno = spy_array.find_town_spy(town_recno, raceId, misc.random(race_spy_count_array[raceId-1])+1 );

		spy_array.del_spy(spyRecno);
	}

	//---- killing civilian people decreases loyalty -----//

	if( nation_recno && attackerNationRecno )					// your people's loyalty decreases because you cannot protect them.
		nation_array[nation_recno]->civilian_killed(raceId, -1);		// but only when your units are killed by enemies, neutral disasters are not counted

	if( attackerNationRecno )        //	the attacker's people's loyalty decreases because of the killing actions.
		nation_array[attackerNationRecno]->civilian_killed(raceId, 2);		// the nation is the attacking one

	// -------- sound effect ---------//

	se_res.sound( center_x, center_y, 1, 'R', raceId, "DIE" );

	//-------- decrease population now --------//

	dec_pop( raceId, 0 );		// 0-doesn't have a job
}
//---------- End of function Town::kill_town_people -----//


//--------- Begin of function Town::unjob_town_people -------//
//
// Make one town people of the specific race jobless.
// Workers are processed first, overseers are processed next.
//
// <int> raceId		  - race id.
// <int> unjobOverseer - whether unjob overseer or not
// <int> killOverseer  - kill the overseer if possible
//
// return: <int> a town person has been made jobless
//
int Town::unjob_town_people(int raceId, int unjobSpy, int unjobOverseer, int killOverseer)
{
	//---- if no jobless people, workers will then get killed -----//

	int 	i, workerId;
	Firm* firmPtr;
	Worker* workerPtr;

	int racePop = jobless_race_pop_array[raceId-1];

	for( i=linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[linked_firm_array[i]];

		//------- scan for workers -----------//

		workerPtr=firmPtr->worker_array;

		for( workerId=1 ; workerId<=firmPtr->worker_count ; workerId++, workerPtr++ )
		{
			if( config_adv.fix_town_unjob_worker && !unjobSpy && workerPtr->spy_recno )
				continue;

			//--- if the worker lives in this town ----//

			if( workerPtr->race_id == raceId &&
				 workerPtr->town_recno == town_recno )
			{
				if( !firmPtr->resign_worker(workerId) && !config_adv.fix_town_unjob_worker )
					return 0;

				err_when(population>MAX_TOWN_POPULATION);
				err_when( jobless_race_pop_array[raceId-1] != racePop+1 );
				return jobless_race_pop_array[raceId-1] == racePop+1;
			}
		}
	}

	//----- if no worker killed, try to kill overseers ------//

	if( unjobOverseer )
	{
		Unit* overseerUnit;

		for( i=linked_firm_count-1 ; i>=0 ; i-- )
		{
			firmPtr = firm_array[linked_firm_array[i]];

			//------- scan for overseer -----------//

			if( firmPtr->overseer_recno )
			{
				//--- if the overseer lives in this town ----//

				overseerUnit = unit_array[firmPtr->overseer_recno];

				if( overseerUnit->race_id == raceId &&
					 firmPtr->overseer_town_recno == town_recno )
				{
					if(killOverseer)
						firmPtr->kill_overseer();
					else
					{
						int overseerUnitRecno = firmPtr->overseer_recno;
						Unit *unitPtr = unit_array[overseerUnitRecno];
						firmPtr->assign_overseer(0);
						if(!unit_array.is_deleted(overseerUnitRecno) && unitPtr->is_visible())
							unit_array.disappear_in_town(overseerUnitRecno, town_recno);
					}
					err_when( jobless_race_pop_array[raceId-1] != racePop+1 );
					return 1;
				}
			}
		}
	}

	return 0;
}
//---------- End of function Town::unjob_town_people -----//


//--------- Begin of function Town::distribute_demand ---------//
//
// Distribute demands among
//
void Town::distribute_demand()
{
	#define MAX_ACTIVE_MARKET_COUNT     20

	//------------ define struct MarketInfo --------------//

	struct MarketGoodsInfo
	{
		FirmMarket* market_ptr[MAX_ACTIVE_MARKET_COUNT];
		float			total_supply;
		float			total_own_supply;
		short			market_count;
	};

   //------ scan for a firm to input raw materials --------//

   int					i, j;
	Firm*					firmPtr;
	MarketGoodsInfo	marketGoodsInfoArray[MAX_PRODUCT];
	MarketGoodsInfo*  marketGoodsInfo;
	float					thisSupply;
	MarketGoods*		marketGoods;

	memset( marketGoodsInfoArray, 0, sizeof(marketGoodsInfoArray) );

	//------- count the no. of market place that are near to this town ----//

	for( int linkedFirmId=0 ; linkedFirmId<linked_firm_count ; linkedFirmId++ )
	{
		firmPtr = firm_array[ linked_firm_array[linkedFirmId] ];

		if( firmPtr->firm_id != FIRM_MARKET )
			continue;

		if( linked_firm_enable_array[linkedFirmId] != LINK_EE )
			continue;

		firmPtr = firm_array[linked_firm_array[linkedFirmId]];

		//---------- process market -------------//

		for( i=0 ; i<MAX_PRODUCT ; i++ )
		{
			marketGoods = ((FirmMarket*)firmPtr)->market_product_array[i];
			marketGoodsInfo = marketGoodsInfoArray+i;

			if( marketGoods && marketGoodsInfo->market_count < MAX_ACTIVE_MARKET_COUNT )
			{
				thisSupply  = marketGoods->stock_qty;

				marketGoodsInfo->market_ptr[marketGoodsInfo->market_count] = (FirmMarket*) firmPtr;

				marketGoodsInfo->total_supply += thisSupply;

				if( firmPtr->nation_recno == nation_recno )				// vars for later use, so that towns will always try to buy goods from their own markets first.
					marketGoodsInfo->total_own_supply += thisSupply;

				marketGoodsInfo->market_count++;
			}
		}
	}

	//-- set the monthly demand of the town on each product --//

	float townDemand = (float) jobless_population * (float) PEASANT_GOODS_MONTH_DEMAND
							 + (float) worker_population() * (float) WORKER_GOODS_MONTH_DEMAND;

	float ownShareDemand;		// the share of demand for own markets

	//---------- sell goods now -----------//

	FirmMarket* firmMarket;

	for( i=0 ; i<MAX_PRODUCT ; i++ )
	{
		marketGoodsInfo = marketGoodsInfoArray+i;

		for( j=0 ; j<marketGoodsInfo->market_count ; j++ )
		{
			//----------------------------------//
			//
			// If the totalSupply < town demand:
			// a market's demand = its_supply + (town_demand-totalSupply) / market_count
			//
			// If the totalSupply > town demand:
			// a market's demand = town_demand * its_supply / totalSupply
			//
			//----------------------------------//

			firmMarket = marketGoodsInfo->market_ptr[j];

			marketGoods = firmMarket->market_product_array[i];

			if( marketGoods )
			{
				//---- if the demand is larger than the supply -----//

				if( marketGoodsInfo->total_supply <= townDemand )
				{
					marketGoods->month_demand += marketGoods->stock_qty +
														 (townDemand - marketGoodsInfo->total_supply)
														 / marketGoodsInfo->market_count;				// evenly distribute the excessive demand on all markets
				}
				else //---- if the supply is larger than the demand -----//
				{
					//--- towns always try to buy goods from their own markets first ---//

					ownShareDemand = MIN(townDemand, marketGoodsInfo->total_own_supply);

					if( firmMarket->nation_recno == nation_recno )
					{
						if (marketGoodsInfo->total_own_supply > 0.0f) // if total_own_supply is 0 then ownShareDemand is also 0 and we put no demand on the product
							marketGoods->month_demand += ownShareDemand * marketGoods->stock_qty / marketGoodsInfo->total_own_supply;
					}
					else
					{
						// Note: total_supply > 0.0f, because else the first case above (demand larger than supply) will be triggered
						marketGoods->month_demand += (townDemand-ownShareDemand) * marketGoods->stock_qty / marketGoodsInfo->total_supply;
					}
				}
			}
		}
	}
}
//----------- End of function Town::distribute_demand -----------//


//------- Begin of function Town::setup_link ---------//
//
void Town::setup_link()
{
	//-----------------------------------------------------------------------------//
	// check the connected firms location and structure if ai_link_checked is true
	//-----------------------------------------------------------------------------//
	if(ai_town)
		ai_link_checked = 0;

	//----- build town-to-firm link relationship -------//

	int   firmRecno, defaultLinkStatus;
	Firm* firmPtr;
	FirmInfo* firmInfo;

	linked_firm_count = 0;

	for( firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
	{
		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr  = firm_array[firmRecno];
		firmInfo = firm_res[firmPtr->firm_id];

		if( !firmInfo->is_linkable_to_town )
			continue;

		//---------- check if the firm is close enough to this firm -------//

		if( misc.points_distance( firmPtr->center_x, firmPtr->center_y,
			 center_x, center_y ) > EFFECTIVE_FIRM_TOWN_DISTANCE )
		{
			continue;
		}

		//------ check if both are on the same terrain type ------//

		if( (world.get_loc(firmPtr->center_x, firmPtr->center_y)->is_plateau()==1)
			 != (world.get_loc(center_x, center_y)->is_plateau()==1) )
		{
			continue;
		}

		//----- check for empty link slots -----//

		if( linked_firm_count >= MAX_LINKED_FIRM_TOWN )
		{
			err_here();
			break;
		}

		if( firmPtr->linked_town_count >= MAX_LINKED_FIRM_TOWN )
		{
			err_here();
			continue;  // linking must be mutual so skip this firm
		}

		//------- determine the default link status ------//

		if( firmPtr->nation_recno == nation_recno )   // if the two firms are of the same nation, get the default link status which is based on the types of the firms
			defaultLinkStatus = LINK_EE;
		else
			defaultLinkStatus = LINK_DD;			//	if the two firms are of different nations, default link status is both side disabled

		//----- a town cannot disable a camp's link to it ----//

		if( firmPtr->firm_id==FIRM_CAMP ) 		// for capturing the town
			defaultLinkStatus = LINK_EE;

		//-------- add the link now -------//

		linked_firm_array[linked_firm_count] = firmRecno;
		linked_firm_enable_array[linked_firm_count] = defaultLinkStatus;

		linked_firm_count++;

		// now link from the firm's side
		if( defaultLinkStatus==LINK_ED )		// Reverse the link status for the opposite linker
			defaultLinkStatus=LINK_DE;

		else if( defaultLinkStatus==LINK_DE )
			defaultLinkStatus=LINK_ED;

		firmPtr->linked_town_array[firmPtr->linked_town_count] = town_recno;
		firmPtr->linked_town_enable_array[firmPtr->linked_town_count] = defaultLinkStatus;

		firmPtr->linked_town_count++;
		if(firmPtr->firm_ai)
			firmPtr->ai_link_checked = 0;
	}

	//----- build town-to-town link relationship -------//

	linked_town_count = 0;

	int   townRecno;
	Town* townPtr;

	for( townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
	{
		if( town_array.is_deleted(townRecno) || townRecno==town_recno )
			continue;

		townPtr = town_array[townRecno];

		//------ check if the town is close enough to this firm -------//

		if( misc.points_distance( townPtr->center_x, townPtr->center_y,
			 center_x, center_y ) > EFFECTIVE_TOWN_TOWN_DISTANCE )
		{
			continue;
		}

		//------ check if both are on the same terrain type ------//

		if( (world.get_loc(townPtr->center_x, townPtr->center_y)->is_plateau()==1)
			 != (world.get_loc(center_x, center_y)->is_plateau()==1) )
		{
			continue;
		}

		//----- check for empty link slots -----//

		if( linked_town_count >= MAX_LINKED_TOWN_TOWN )
		{
			err_here();
			break;
		}

		if( townPtr->linked_town_count >= MAX_LINKED_TOWN_TOWN )
		{
			err_here();
			continue;  // linking must be mutual so skip this town
		}

		//------- determine the default link status ------//

		defaultLinkStatus = LINK_EE;

		//-------- add the link now -------//

		linked_town_array[linked_town_count] = townRecno;
		linked_town_enable_array[linked_town_count] = defaultLinkStatus;

		linked_town_count++;

		// now link from the other town's side
		if( defaultLinkStatus==LINK_ED )		// Reverse the link status for the opposite linker
			defaultLinkStatus=LINK_DE;

		else if( defaultLinkStatus==LINK_DE )
			defaultLinkStatus=LINK_ED;

		townPtr->linked_town_array[townPtr->linked_town_count] = town_recno;
		townPtr->linked_town_enable_array[townPtr->linked_town_count] = defaultLinkStatus;

		townPtr->linked_town_count++;
		if(townPtr->ai_town)
			townPtr->ai_link_checked = 0;
	}
}
//-------- End of function Town::setup_link -----------//


//------- Begin of function Town::release_link ---------//
//
void Town::release_link()
{
   int i;
	Firm *firmPtr;
	Town *townPtr;

   //------ release linked firms ------//

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[linked_firm_array[i]];
		firmPtr->release_town_link(town_recno);
		
		if(firmPtr->firm_ai)
			firmPtr->ai_link_checked = 0;
   }

   //------ release linked towns ------//

   for( i=0 ; i<linked_town_count ; i++ )
   {
		townPtr = town_array[linked_town_array[i]];
		townPtr->release_town_link(town_recno);
		
		if(townPtr->ai_town)
			townPtr->ai_link_checked = 0;
   }
}
//-------- End of function Town::release_link -----------//


//------- Begin of function Town::release_firm_link ---------//
//
void Town::release_firm_link(int releaseFirmRecno)
{
	//-----------------------------------------------------------------------------//
	// check the connected firms location and structure if ai_link_checked is true
	//-----------------------------------------------------------------------------//
	if(ai_town)
		ai_link_checked = 0;

	for( int i=0 ; i<linked_firm_count ; i++ )
   {
      if( linked_firm_array[i] == releaseFirmRecno )
      {
			err_when( linked_firm_count > MAX_LINKED_FIRM_TOWN );

			misc.del_array_rec( linked_firm_array		  , linked_firm_count, sizeof(linked_firm_array[0]), i+1 );
			misc.del_array_rec( linked_firm_enable_array, linked_firm_count, sizeof(linked_firm_enable_array[0]), i+1 );
			linked_firm_count--;
			return;
		}
	}

	err_here();
}
//------- End of function Town::release_firm_link ---------//


//------- Begin of function Town::release_town_link ---------//
//
void Town::release_town_link(int releaseTownRecno)
{
	//-----------------------------------------------------------------------------//
	// check the connected firms location and structure if ai_link_checked is true
	//-----------------------------------------------------------------------------//
	if(ai_town)
		ai_link_checked = 0;

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		if( linked_town_array[i] == releaseTownRecno )
		{
			err_when( linked_town_count > MAX_LINKED_TOWN_TOWN );

			misc.del_array_rec( linked_town_array		  , linked_town_count, sizeof(linked_town_array[0]), i+1 );
			misc.del_array_rec( linked_town_enable_array, linked_town_count, sizeof(linked_town_enable_array[0]), i+1 );
			linked_town_count--;
			return;
		}
	}

	err_here();
}
//------- End of function Town::release_town_link ---------//


//------- Begin of function Town::toggle_firm_link ---------//
//
// Toggle the firm link of the current town.
//
// <int> linkId - id. of the link
// <int> toggleFlag - 1-enable, 0-disable
// <char> remoteAction - remote action type 
// [int] setBoth - if this is 1, it will set the link to either LINK_EE or LINK_DD (and no LINK_ED or LINK_DD)
//						 if this is -1, the only one side will be set even though the nation recno of the firm and town are the same
//					    (default: 0)
//
void Town::toggle_firm_link(int linkId, int toggleFlag, char remoteAction, int setBoth)
{
	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <town recno> <link Id> <toggle Flag>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_TOGGLE_LINK_FIRM, 3*sizeof(short));
		shortPtr[0] = town_recno;
		shortPtr[1] = linkId;
		shortPtr[2] = toggleFlag;
		return;
	}

	Firm* linkedFirm = firm_array[linked_firm_array[linkId-1]];
	int 	linkedNationRecno = linkedFirm->nation_recno;

	int sameNation = linkedNationRecno == nation_recno ||		// if one of the linked end is an indepdendent firm/nation, consider this link as a single nation link
						  linkedNationRecno == 0 ||
						  nation_recno == 0;

	if( toggleFlag )
	{
		if( (sameNation && setBoth==0) || setBoth==1 )		// 0 if setBoth == -1
			linked_firm_enable_array[linkId-1] = LINK_EE;
		else
			linked_firm_enable_array[linkId-1] |= LINK_ED;
	}
	else
	{
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_firm_enable_array[linkId-1] = LINK_DD;
		else
			linked_firm_enable_array[linkId-1] &= ~LINK_ED;
	}

	//------ set the linked flag of the opposite firm -----//

	Firm* firmPtr = firm_array[ linked_firm_array[linkId-1] ];
	int   i;

	for(i=firmPtr->linked_town_count-1; i>=0; i--)
	{
		if( firmPtr->linked_town_array[i] == town_recno )
		{
			if( toggleFlag )
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					firmPtr->linked_town_enable_array[i]  = LINK_EE;
				else
					firmPtr->linked_town_enable_array[i] |= LINK_DE;
			}
			else
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					firmPtr->linked_town_enable_array[i]  = LINK_DD;
				else
					firmPtr->linked_town_enable_array[i] &= ~LINK_DE;
			}

			break;
		}
	}

	//-------- update the town's influence --------//

	if( nation_recno==0 )
		update_target_resistance();

	//--- redistribute demand if a link to market place has been toggled ---//

	if( linkedFirm->firm_id == FIRM_MARKET )
		town_array.distribute_demand();
}
//-------- End of function Town::toggle_firm_link ---------//


//------- Begin of function Town::toggle_town_link ---------//
//
// Toggle the town link of the current town.
//
// NOTE: This function is not used
//
// <int> linkId - id. of the link
// <int> toggleFlag - 1-enable, 0-disable
// [int] setBoth - if this is 1, it will set the link to either LINK_EE or LINK_DD (and no LINK_ED or LINK_DD)
//						 if this is -1, the only one side will be set even though the nation recno of the firm and town are the same
//					    (default: 0)
//
void Town::toggle_town_link(int linkId, int toggleFlag, char remoteAction, int setBoth)
{
	// Function is unused, and not updated to support town networks.
	return;

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <town recno> <link Id> <toggle Flag>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_TOGGLE_LINK_TOWN, 3*sizeof(short));
		shortPtr[0] = town_recno;
		shortPtr[1] = linkId;
		shortPtr[2] = toggleFlag;
		return;
	}

	int linkedNationRecno = town_array[linked_town_array[linkId-1]]->nation_recno == nation_recno;

	int sameNation = linkedNationRecno == nation_recno ||		// if one of the linked end is an indepdendent firm/nation, consider this link as a single nation link
						  linkedNationRecno == 0 ||
						  nation_recno == 0;

	if( toggleFlag )
	{
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_town_enable_array[linkId-1]  = LINK_EE;
		else
			linked_town_enable_array[linkId-1] |= LINK_ED;
	}
	else
	{
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_town_enable_array[linkId-1]  = LINK_DD;
		else
			linked_town_enable_array[linkId-1] &= ~LINK_ED;
	}

	//------ set the linked flag of the opposite town -----//

	Town* townPtr = town_array[ linked_town_array[linkId-1] ];
	int   i;

	for(i=townPtr->linked_town_count-1; i>=0; i--)
	{
		if( townPtr->linked_town_array[i] == town_recno )
		{
			if( toggleFlag )
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					townPtr->linked_town_enable_array[i]  = LINK_EE;
				else
					townPtr->linked_town_enable_array[i] |= LINK_DE;
			}
			else
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					townPtr->linked_town_enable_array[i]  = LINK_DD;
				else
					townPtr->linked_town_enable_array[i] &= ~LINK_DE;
			}

			break;
		}
	}
}
//-------- End of function Town::toggle_town_link ---------//


//-------- Begin of function Town::linked_active_camp_count ------//
//
// No. of linked military camp to this town which has a general in it.
//
int Town::linked_active_camp_count()
{
	int 	i, linkedCount=0;
	Firm* firmPtr;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_enable_array[i] == LINK_EE )
		{
			firmPtr = firm_array[linked_firm_array[i]];

			if( firmPtr->firm_id == FIRM_CAMP &&
				 firmPtr->overseer_recno )
			{
				linkedCount++;
			}
		}
	}

	return linkedCount;
}
//---------- End of function Town::linked_active_camp_count --------//


//------- Begin of function Town::auto_defense ---------//
//
void Town::auto_defense(short targetRecno)
{
	Firm *firmPtr;
	FirmCamp *campPtr;
	short townRecno = town_recno;

	for(int i=linked_firm_count-1; i>=0; i--)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));
		firmPtr = firm_array[linked_firm_array[i]];

		if(firmPtr->nation_recno!=nation_recno || firmPtr->firm_id!=FIRM_CAMP)
			continue;

		//-------------------------------------------------------//
		// the firm is a military camp of our nation
		//-------------------------------------------------------//
		campPtr = firmPtr->cast_to_FirmCamp();
		campPtr->defense(targetRecno);

		if(town_array.is_deleted(townRecno))
			break; // the last unit in the town has be mobilized
	}
}
//-------- End of function Town::auto_defense ---------//


//-------- Begin of function Town::cancel_train_unit ------//
void Town::cancel_train_unit()
{
	if( train_unit_recno )
	{
		//### begin alex 17/3 ###//
		//unit_array.disappear_in_town(train_unit_recno, town_recno);
		//train_unit_recno = 0;

		err_when(train_unit_recno==0);
		Unit *unitPtr = unit_array[train_unit_recno];
		if(unitPtr->spy_recno && unitPtr->skill.skill_id) // check whether the unit is already a spy before training
		{
			spy_array[unitPtr->spy_recno]->set_place(SPY_TOWN, town_recno);
			unitPtr->spy_recno = 0;		// reset it so Unit::deinit() won't delete the spy
		}

		unit_array.disappear_in_town(train_unit_recno, town_recno);
		train_unit_recno = 0;
		//#### end alex 17/3 ####//
	}
}
//----------- End of function Town::cancel_train_unit -----------//


//-------- Begin of function Town::auto_set_layout ------//
//
void Town::auto_set_layout()
{
/*
	//------ debugging code -----//

	static int lastLayoutId=0;

	if( ++lastLayoutId > town_res.town_layout_count )
		lastLayoutId=1;

	layout_id = lastLayoutId;
*/
	//------------------//

	layout_id = think_layout_id();		

	err_when( !layout_id );

	TownLayout* townLayout = town_res.get_layout(layout_id);
	TownSlot*   firstTownSlot = town_res.get_slot(townLayout->first_slot_recno);
	short			raceNeedBuildCount[MAX_RACE];

	memset( slot_object_id_array, 0, sizeof(slot_object_id_array) );
	memset( max_race_pop_array , 0, sizeof(max_race_pop_array) );
	memset( raceNeedBuildCount, 0, sizeof(raceNeedBuildCount) );

	int i;
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] > 0 )
			raceNeedBuildCount[i] += (race_pop_array[i]-1)/POPULATION_PER_HOUSE+1;
	}

	//--- assign the first house to each race, each present race will at least have one house ---//

	int firstRaceId = random_race();		// random match
	int raceId = firstRaceId;

	for( i=0 ; i<townLayout->slot_count ; i++ )
	{
		if( firstTownSlot[i].build_type == TOWN_OBJECT_HOUSE )
		{
			int loopCount=0;

			while(1)	// next race
			{
				if( ++raceId > MAX_RACE )
					raceId = 1;

				if( raceId == (firstRaceId==MAX_RACE)?1:(firstRaceId+1) )		// finished the first house for all races
					goto label_distribute_house;

				if( raceNeedBuildCount[raceId-1] > 0 )  // if this race need buildings, skip all races that do not need buildings
					break;

				err_when( loopCount++ > 1000 );
			}

			slot_object_id_array[i] = town_res.scan_build( townLayout->first_slot_recno+i, raceId);

			raceNeedBuildCount[raceId-1]--;
			max_race_pop_array[raceId-1] += POPULATION_PER_HOUSE;
		}
	}

	err_when( raceId != firstRaceId );		// some races are not assigned with a house

	//------- distribute the remaining houses -------//

label_distribute_house:

	int bestRaceId, maxNeedBuildCount;

	for( i=0 ; i<townLayout->slot_count ; i++ )
	{
		if( firstTownSlot[i].build_type == TOWN_OBJECT_HOUSE && !slot_object_id_array[i] )
		{
			bestRaceId=0;
			maxNeedBuildCount=0;

			for( raceId=1 ; raceId<=MAX_RACE ; raceId++ )
			{
				if( raceNeedBuildCount[raceId-1] > maxNeedBuildCount )
				{
					bestRaceId = raceId;
					maxNeedBuildCount = raceNeedBuildCount[raceId-1];
				}
			}

			if( !bestRaceId )		// all races have assigned with their needed houses
				break;

			slot_object_id_array[i] = town_res.scan_build( townLayout->first_slot_recno+i, bestRaceId);
			raceNeedBuildCount[bestRaceId-1]--;
			max_race_pop_array[bestRaceId-1] += POPULATION_PER_HOUSE;
		}
	}

	//------- set plants in the town layout -------//

	for( i=0 ; i<townLayout->slot_count ; i++ )
	{
		switch(firstTownSlot[i].build_type)
		{
			case TOWN_OBJECT_PLANT:
				slot_object_id_array[i] = plant_res.scan(0, 'T', 0);    // 'T'-town only, 1st 0-any zone area, 2nd 0-any terain type, 3rd-age level
				break;

			case TOWN_OBJECT_FARM:
				slot_object_id_array[i] = firstTownSlot[i].build_code;
				err_when( slot_object_id_array[i] < 1 || slot_object_id_array[i] > 9 );
				break;

			case TOWN_OBJECT_HOUSE:
				if( !slot_object_id_array[i] )
					slot_object_id_array[i] = town_res.scan_build( townLayout->first_slot_recno+i, random_race() );
				break;
		}
	}
}
//---------- End of function Town::auto_set_layout --------//


//-------- Begin of function Town::think_layout_id ------//
//
int Town::think_layout_id()
{
	int i, countDiff;
	int needBuildCount=0;		// basic buildings needed
	int extraBuildCount=0;		// extra buildings needed beside the basic one

	//---- count the needed buildings of each race ----//

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] == 0 )
			continue;

		needBuildCount++;		// essential buildings needed

		if( race_pop_array[i] > POPULATION_PER_HOUSE )		// extra buildings, which are not necessary, but will look better if the layout plan fits with this number
			extraBuildCount += (race_pop_array[i]-POPULATION_PER_HOUSE-1)/POPULATION_PER_HOUSE+1;
	}

	//---------- scan the town layout ---------//

	int 		   layoutId;
	TownLayout* townLayout;

	for( layoutId=town_res.town_layout_count ; layoutId>0 ; layoutId-- )	// scan from the most densed layout to the least densed layout
	{
		townLayout = town_res.get_layout(layoutId);

		//--- if this plan has less than the essential need ---//

		countDiff = townLayout->build_count - (needBuildCount+extraBuildCount);

		if( countDiff==0 )		// the match is perfect, return now
			break;

		if( countDiff < 0 )		// since we scan from the most densed town layout to the least densed one, if cannot find anyone matched now, there won't be any in the lower positions of the array
		{
			layoutId = town_res.town_layout_count;
			break;
		}
	}

	err_when( layoutId==0 );

	//--- if there are more than one layout with the same number of building, pick one randomly ---//

	int layoutBuildCount = town_res.get_layout(layoutId)->build_count;
	int layoutId2;

	for( layoutId2=layoutId-1 ; layoutId2>0 ; layoutId2-- )
	{
		townLayout = town_res.get_layout(layoutId2);

		if( layoutBuildCount != townLayout->build_count )
			break;
	}

	layoutId2++;		// the lowest layout id. that has the same no. of buildings

	err_when( layoutId2<1 || layoutId2>town_res.town_layout_count );

	//------- return the result layout id -------//

	return layoutId2 + misc.random(layoutId-layoutId2+1);
}
//---------- End of function Town::think_layout_id --------//


//-------- Begin of function Town::average_loyalty ------//
//
int Town::average_loyalty()
{
	int i, totalLoyalty=0;

	for( i=0 ; i<MAX_RACE ; i++ )
		totalLoyalty += (int) race_loyalty_array[i] * race_pop_array[i];

	return totalLoyalty / population;
}
//---------- End of function Town::average_loyalty --------//


//-------- Begin of function Town::average_target_loyalty ------//
//
int Town::average_target_loyalty()
{
	int i, totalLoyalty=0;

	for( i=0 ; i<MAX_RACE ; i++ )
		totalLoyalty += (int) race_target_loyalty_array[i] * race_pop_array[i];

	return totalLoyalty / population;
}
//---------- End of function Town::average_target_loyalty --------//


//------- Begin of function Town::average_resistance --------//
//
// Return the average resistance of all the races in the town
// against a specific nation.
//
int Town::average_resistance(int nationRecno)
{
	int   thisPop;
	float totalResistance=(float)0;

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		thisPop = race_pop_array[i];

		if( thisPop > 0 )
			totalResistance += race_resistance_array[i][nationRecno-1] * thisPop;
	}

	return int(totalResistance / population);
}
//-------- End of function Town::average_resistance ---------//


//------- Begin of function Town::average_target_resistance --------//
//
// Return the average target resistance of all the races in the town
// against a specific nation.
//
int Town::average_target_resistance(int nationRecno)
{
	int   thisPop, t, totalResistance=0;

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		thisPop = race_pop_array[i];

		if( thisPop > 0 )
		{
			t = race_target_resistance_array[i][nationRecno-1];

			if( t >= 0 ) 		// -1 means no target
				totalResistance += t * thisPop;
			else
				totalResistance += (int) race_resistance_array[i][nationRecno-1] * thisPop;
		}
	}

	return int(totalResistance / population);
}
//-------- End of function Town::average_target_resistance ---------//


//-------- Begin of function Town::can_recruit ------//
//
int Town::can_recruit(int raceId)
{
	//----------------------------------------------------//
	// Cannot recruit when you have none of your own camps
	// linked to this town, but your enemies have camps
	// linked to it.
	//----------------------------------------------------//

	if( !has_linked_own_camp && has_linked_enemy_camp )
		return 0;

	if( recruitable_race_pop(raceId,1)==0 )
		return 0;

	err_when( recruitable_race_pop(raceId,1) < 0 );

	//---------------------------------//

	int minRecruitLoyalty = MIN_RECRUIT_LOYALTY;

	//--- for the AI, only recruit if the loyalty still stay at 30 after recruiting the unit ---//

	if(ai_town && nation_recno)
		minRecruitLoyalty += 3+recruit_dec_loyalty(raceId, 0);		// 0-don't actually decrease it, just return the loyalty to be decreased.

	return race_loyalty_array[raceId-1] >= minRecruitLoyalty;
}
//---------- End of function Town::can_recruit --------//


//-------- Begin of function Town::can_train ------//
//
int Town::can_train(int raceId)
{
	int recruitableCount = jobless_race_pop_array[raceId-1];

	return has_linked_own_camp && recruitableCount > 0 &&
			 nation_array[nation_recno]->cash > TRAIN_SKILL_COST;
}
//---------- End of function Town::can_train --------//


//-------- Begin of function Town::pick_random_race ------//
//
// Randonly pick a race of town people who live in the town.
//
// <int> pickNonRecruitableAlso - whether also pick units that have jobs.
// <int> pickSpyFlag            - whether will pick spies or not.
//
// return: <int> id. of the race picked.
//					  0 - no race picked.
//
int Town::pick_random_race(int pickNonRecruitableAlso, int pickSpyFlag)
{
	int totalPop;

	if( pickNonRecruitableAlso )
		totalPop = population;
	else
	{
		totalPop = jobless_population - (train_unit_recno>0);

		if( !pickSpyFlag )		// if don't pick spies
		{
			for( int i=0 ; i<MAX_RACE ; i++ )
				totalPop -= race_spy_count_array[i];

			if( totalPop == -1 )		// it can be -1 if the unit being trained is a spy
				totalPop = 0;
		}

		err_when( totalPop < 0 );
	}

	if( totalPop==0 )
		return 0;

	int randomPersonId = misc.random(totalPop)+1;
	int popSum=0;

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		if( pickNonRecruitableAlso )
			popSum += race_pop_array[i];
		else
			popSum += recruitable_race_pop(i+1, pickSpyFlag);

		if( randomPersonId <= popSum )
			return i+1;
	}

	err_here();
	return 0;
}
//---------- End of function Town::pick_random_race --------//


//-------- Begin of function Town::get_most_populated_race ------//
//
// <int&> raceId1, raceId2 - return the id. of the most and
//									  2nd most populated races.
//
void Town::get_most_populated_race(int& mostRaceId1, int& mostRaceId2)
{
	//--- find the two races with most population in the town ---//

	int	racePop;
	int   mostRacePop1=0, mostRacePop2=0;

	mostRaceId1=0;
	mostRaceId2=0;

	if( population==0 )
		return;

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		racePop = race_pop_array[i];

		if( racePop==0 )
			continue;

		if( racePop >= mostRacePop1 )
		{
			mostRacePop2 = mostRacePop1;
			mostRacePop1 = racePop;

			mostRaceId2 = mostRaceId1;
			mostRaceId1 = i+1;
		}
		else if( racePop >= mostRaceId2 )
		{
			mostRacePop2 = racePop;
			mostRaceId2  = i+1;
		}
	}
}
//---------- End of function Town::get_most_populated_race --------//


//-------- Begin of function Town::majority_race ------//
//
int Town::majority_race()
{
	int mostRaceCount=0, mostRaceId=0;

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] > mostRaceCount )
		{
			mostRaceCount = race_pop_array[i];
			mostRaceId 	  = i+1;
		}
	}

	return mostRaceId;
}
//---------- End of function Town::majority_race --------//


//-------- Begin of function Town::recruitable_race_pop ------//
//
// Return the number of units can be recruited from this town.
//
// <int> raceId	  - id. of the race to recruit
//
// <int> recruitSpy - whether spies will be recruited or not.
//							 if no, spies are not counted in the
//							 total no. of units that can be recruited.
//
int Town::recruitable_race_pop(int raceId, int recruitSpy)
{
	err_when( raceId<1 || raceId>MAX_RACE );

	short recruitableCount = jobless_race_pop_array[raceId-1];
	
	if( train_unit_recno && unit_array[train_unit_recno]->race_id==raceId )
		recruitableCount--;

	if( !recruitSpy )
	{
		recruitableCount -= race_spy_count_array[raceId-1];

		if( recruitableCount == -1 )		// it may have been reduced twice if the unit being trained is a spy 
			recruitableCount = 0;
	}		

	err_when( recruitableCount < 0 );

	return recruitableCount;
}
//---------- End of function Town::recruitable_race_pop --------//


//------ Begin of function Town::verify_slot_object_id_array ------//
//
// This function is for debugging only.
//
void Town::verify_slot_object_id_array()
{
	TownLayout* townLayout = town_res.get_layout(layout_id);
	TownSlot*   townSlot   = town_res.get_slot(townLayout->first_slot_recno);

	for( int i=0 ; i<townLayout->slot_count ; i++, townSlot++ )
	{
		//----- build_type==0 if plants -----//

		switch(townSlot->build_type)
		{
			//----- build_type>0 if town buildings -----//

			case TOWN_OBJECT_HOUSE:
				town_res.get_build( slot_object_id_array[i] );
				break;

			case TOWN_OBJECT_PLANT:
				plant_res.get_bitmap( slot_object_id_array[i] );
				break;

			case TOWN_OBJECT_FARM:
				err_when( slot_object_id_array[i]<1 || slot_object_id_array[i]>9 );
				break;
		}
	}
}
//-------- End of function Town::verify_slot_object_id_array ------//


//----- Begin of function Town::set_auto_collect_tax_loyalty -----//
//
// Note:
//
// Auto collect tax loyalty must always be higher than the auto grant
// loyalty. If the player has set it incorrectly, the program
// will automatically adjust it. e.g. If the player sets the auto
// grant level to 80 while the auto tax level is currently 60,
// the program will adjust auto tax level to 90. And if the player
// sets the auto grant level to 100, auto tax will be disabled.
//
void Town::set_auto_collect_tax_loyalty(int loyaltyLevel)
{
	auto_collect_tax_loyalty = loyaltyLevel;

	if( loyaltyLevel && auto_grant_loyalty >= auto_collect_tax_loyalty )
	{
		auto_grant_loyalty = auto_collect_tax_loyalty-10;
	}
}
//------ End of function Town::set_auto_collect_tax_loyalty -----//


//----- Begin of function Town::set_auto_grant_loyalty -----//
//
void Town::set_auto_grant_loyalty(int loyaltyLevel)
{
	auto_grant_loyalty = loyaltyLevel;

	if( loyaltyLevel && auto_grant_loyalty >= auto_collect_tax_loyalty )
	{
		auto_collect_tax_loyalty = auto_grant_loyalty+10;

		if( auto_collect_tax_loyalty > 100 )
			auto_collect_tax_loyalty = 0;					// disable auto collect tax if it's over 100
	}
}
//------ End of function Town::set_auto_grant_loyalty -----//


//------ Begin of function Town::update_quality_of_life -------//
//
// Quality of life is determined by:
//
// - The provision of goods to the villagers. A more constant
//	  supply and a bigger variety of goods give to high quality of life.
//
void Town::update_quality_of_life()
{
	Firm* 		firmPtr;
	FirmMarket* firmMarket;

	//--- calculate the estimated total purchase from this town ----//

	float townDemand = (float) jobless_population * (float) PEASANT_GOODS_MONTH_DEMAND
							 + (float) worker_population() * (float) WORKER_GOODS_MONTH_DEMAND;

	float totalPurchase = (float) 0;

	for( int i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_enable_array[i] != LINK_EE )
			continue;

		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->firm_id != FIRM_MARKET )
			continue;

		firmMarket = (FirmMarket*) firmPtr;

		//-------------------------------------//

		MarketGoods* marketGoods = firmMarket->market_goods_array;

		for( int j=0 ; j<MAX_MARKET_GOODS ; j++, marketGoods++ )
		{
			if( !marketGoods->product_raw_id || marketGoods->month_demand==0 )
				continue;

			float monthSaleQty = marketGoods->sale_qty_30days();

			if( monthSaleQty > marketGoods->month_demand )
			{
				totalPurchase += townDemand;
			}
			else if( marketGoods->month_demand > townDemand )
			{
				totalPurchase += monthSaleQty * townDemand / marketGoods->month_demand;
			}
			else
				totalPurchase += monthSaleQty;
		}
	}

	//------ return the quality of life ------//

	quality_of_life = int( (float)100 * totalPurchase / (townDemand * MAX_PRODUCT) );
}
//----------- End of function Town::update_quality_of_life ---------//


//----- Begin of function Town::has_linked_camp -----//
//
// Return whether there is a camp of the specific nation
// linked to this town.
//
// <int> nationRecno  - recno of the nation.
// <int> needOverseer - whether only count camps with overseers or not.
//
int Town::has_linked_camp(int nationRecno, int needOverseer)
{
	Firm* firmPtr;

	for( int i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->firm_id == FIRM_CAMP &&
			 firmPtr->nation_recno == nationRecno )
		{
			if( !needOverseer || firmPtr->overseer_recno )
				return 1;
		}
	}

	return 0;
}
//------ End of function Town::has_linked_camp -----//


//----- Begin of function Town::can_grant_to_non_own_town -----//
//
// Return whether the given nation is allowed to grant to
// this independent town. Only when the nation has a camp (with
// an overseer) linked to the town, the nation will be able
// to grant the independent village.
//
// <int> grantNationRecno - the recno of the nation that
//									 is going to grant this town.
//
int Town::can_grant_to_non_own_town(int grantNationRecno)
{
	if( nation_recno == grantNationRecno )		// only for independent town
		return 0;

	if( nation_recno == 0 )		// independent town
	{
		return has_linked_camp( grantNationRecno, 1 );		// 1-only count camps with overseers
	}
	else  // for nation town, when the enemy doesn't have camps linked to it and the granting nation has camps linked to it
	{
		return has_linked_camp( nation_recno, 0 )==0 &&		// 0-count camps regardless of the presence of overseers
				 has_linked_camp( grantNationRecno, 1 );		// 1-only count camps with overseers
	}
}
//------ End of function Town::can_grant_to_non_own_town -----//


//----- Begin of function Town::grant_to_non_own_town -----//
//
// <int> grantNationRecno - the recno of the nation that
//									 is going to grant this town.
//
int Town::grant_to_non_own_town(int grantNationRecno, int remoteAction)
{
	if( !can_grant_to_non_own_town(grantNationRecno) )
		return 0;

	Nation* grantNation = nation_array[grantNationRecno];

	if( grantNation->cash < 0 )
		return 0;

	if( !remoteAction && remote.is_enable() )
	{
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_GRANT_INDEPENDENT, 2*sizeof(short) );
		shortPtr[0] = town_recno;
		shortPtr[1] = grantNationRecno;
		return 1;
	}

	//---- calculate the resistance to be decreased -----//

	int resistanceDec = IND_TOWN_GRANT_RESISTANCE_DECREASE - accumulated_enemy_grant_penalty/5;

	resistanceDec = MAX(3, resistanceDec);

	accumulated_enemy_grant_penalty += 10;

	//------ decrease the resistance of the independent villagers ------//

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i]==0 )
			continue;

		//----- if this is an independent town ------//

		if( nation_recno==0 )
		{
			race_resistance_array[i][grantNationRecno-1] -= resistanceDec;

			if( race_resistance_array[i][grantNationRecno-1] < 0 )
				race_resistance_array[i][grantNationRecno-1] = (float) 0;
		}
		else  //----- if this is an nation town ------//
		{
			race_loyalty_array[i] -= resistanceDec;

			if( race_loyalty_array[i] < 0 )
				race_loyalty_array[i] = (float) 0;
		}
	}

	//----------- decrease cash ------------//

	grantNation->add_expense(EXPENSE_GRANT_OTHER_TOWN, (float)population * IND_TOWN_GRANT_PER_PERSON );

	return 1;
}
//------ End of function Town::grant_to_non_own_town -----//


//----- Begin of function Town::update_camp_link -----//
//
// Update the status of links from this town to camps.
//
void Town::update_camp_link()
{
	//--- enable the link of the town's side to all linked camps ---//

	Firm* firmPtr;

	int i;
	for( i=0 ; i<linked_firm_count ; i++ )
	{
		 firmPtr = firm_array[linked_firm_array[i]];

		 if( firmPtr->firm_id != FIRM_CAMP )
			 continue;

		 //--- don't set it if the town and camp both belong to a human player, the player will set it himself ---//

		 if( firmPtr->nation_recno == nation_recno &&
			  nation_recno && !nation_array[nation_recno]->is_ai() )
		 {
			 continue;
		 }

		 //--------------------------------------------//

		 toggle_firm_link( i+1, 1, COMMAND_AUTO );
	}

	//------- update camp link status -------//

	has_linked_own_camp = 0;
	has_linked_enemy_camp = 0;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_enable_array[i] != LINK_EE )
			continue;

		firmPtr = firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id!=FIRM_CAMP || !firmPtr->overseer_recno )
			continue;

		if( firmPtr->nation_recno == nation_recno )
			has_linked_own_camp = 1;
		else
			has_linked_enemy_camp = 1;
	}
}
//------ End of function Town::update_camp_link -----//


//------- Begin of function Town::closest_own_camp -------//
//
// Return the firm recno of the camp that is closest to this town.
//
int Town::closest_own_camp()
{
	Firm *firmPtr;
	int  curDistance, minDistance=1000, closestFirmRecno=0;

	for( int i=linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->firm_id != FIRM_CAMP ||
			 firmPtr->nation_recno != nation_recno )
		{
			continue;
		}

		curDistance = misc.points_distance( center_x, center_y,
						  firmPtr->center_x, firmPtr->center_y );

		if( curDistance < minDistance )
		{
			minDistance = curDistance;
			closestFirmRecno = firmPtr->firm_recno;
		}
	}

	return closestFirmRecno;
}
//-------- End of function Town::closest_own_camp ---------//


//-------- Begin of static function random_race --------//
//
// Uses misc.random() for random race
//
static char random_race()
{
	int num = misc.random(config_adv.race_random_list_max);
	return config_adv.race_random_list[num];
}
//--------- End of static function random_race ---------//
