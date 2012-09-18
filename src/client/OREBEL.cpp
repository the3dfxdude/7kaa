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

//Filename    : OREBEL.CPP
//Description : Object Rebel

#include <OPOWER.h>
#include <OGAME.h>
#include <OUNIT.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <ONEWS.h>
#include <OREBEL.h>
#include <ORACERES.h>
#include <ONATION.h>


//--------- Begin of function RebelArray::RebelArray ----------//

RebelArray::RebelArray() : DynArrayB(sizeof(Rebel*), 10, DEFAULT_REUSE_INTERVAL_DAYS)
{
}
//--------- End of function RebelArray::RebelArary ----------//


//------- Begin of function RebelArray::~RebelArray ----------//
//
RebelArray::~RebelArray()
{
	deinit();
}
//--------- End of function RebelArray::~RebelArray ----------//


//--------- Begin of function RebelArray::init ----------//
//
void RebelArray::init()
{
}
//---------- End of function RebelArray::init ----------//


//--------- Begin of function RebelArray::deinit ----------//
//
void RebelArray::deinit()
{
	if( size()==0 )
		return;

	//----- delete Rebel objects ------//

	Rebel* rebelPtr;

	for( int i=1 ; i<=size() ; i++ )
	{
		rebelPtr = (Rebel*) get_ptr(i);

		if( rebelPtr )
			delete rebelPtr;
	}

	//-------- zap the array -----------//

	zap();
}
//---------- End of function RebelArray::deinit ----------//


//------- Begin of function RebelArray::create_rebel ---------//
//
// Create a rebel group. If there are a rebel group nearby
// with the same objectives, join the rebel group without
// creating a new one.
//
// <int> unitRecno  			 - recno of the rebel leader
// <int> hostileNationRecno - the recno of the now hostile nation which
//										the people rebels from.
// [int] actionMode - rebel action mode (default: REBEL_IDLE)
// [int] actionPara - action parameters (default: 0)
//
// return: <int> rebelRecno - recno of the rebel group
//
int RebelArray::create_rebel(int unitRecno, int hostileNationRecno, int actionMode, int actionPara)
{
	//------------------------------------------//
	//	See if there are a rebel group nearby
	// with the same objectives, join the rebel
	// group without creating a new one.
	//------------------------------------------//

	int 	 rebelRecno = misc.random(rebel_array.size())+1;
	Rebel* rebelPtr;

	for( int i=size() ; i>=1 ; i-- )
	{
		if( ++rebelRecno > rebel_array.size() )
			rebelRecno = 1;

		if( rebel_array.is_deleted(rebelRecno) )
			continue;

		rebelPtr = rebel_array[rebelRecno];

		if( rebelPtr->action_mode == actionMode &&
			 rebelPtr->action_para == actionPara )
		{
			rebelPtr->join(unitRecno);		// join the rebel group
			return rebelRecno;
		}
	}

	//-------- create a new rebel group ---------//

	rebelPtr = new Rebel;

	rebelPtr->leader_unit_recno 	 = unitRecno;
	rebelPtr->action_mode		    = actionMode;
	rebelPtr->action_para		 	 = actionPara;
	rebelPtr->mobile_rebel_count	 = 1;
	rebelPtr->set_hostile_nation(hostileNationRecno);
	
	linkin(&rebelPtr);

	rebelPtr->rebel_recno = recno();

	unit_array[unitRecno]->set_mode( UNIT_MODE_REBEL, recno() );

	return recno();
}
//-------- End of function RebelArray::create_rebel ---------//


//------- Begin of function RebelArray::del_rebel ---------//
//
void RebelArray::del_rebel(int rebelRecno)
{
	Rebel* rebelPtr = operator[](rebelRecno);

	delete rebelPtr;

	linkout(rebelRecno);
}
//-------- End of function RebelArray::del_rebel ---------//


//--------- Begin of function RebelArray::stop_attack_town ---------//
void RebelArray::stop_attack_town(short townRecno)
{
	Rebel	*rebelPtr;
	for(int i=size(); i>0; --i)
	{
		rebelPtr = (Rebel*) get_ptr(i);
		
		if(rebelPtr && rebelPtr->action_mode==REBEL_ATTACK_TOWN)
		{
			if(rebelPtr->action_para==townRecno)
			{
				rebelPtr->set_action(REBEL_IDLE);
				rebelPtr->stop_all_rebel_unit();
				break;
			}
		}
	}
}
//-------- End of function RebelArray::stop_attack_town ---------//


//--------- Begin of function RebelArray::stop_attack_firm ---------//
void RebelArray::stop_attack_firm(short firmRecno)
{
	Rebel	*rebelPtr;
	for(int i=size(); i>0; --i)
	{
		rebelPtr = (Rebel*) get_ptr(i);
		if(rebelPtr && rebelPtr->action_mode==REBEL_ATTACK_FIRM)
		{
			if(rebelPtr->action_para==firmRecno)
			{
				rebelPtr->set_action(REBEL_IDLE);
				rebelPtr->stop_all_rebel_unit();
				return;
			}
		}
	}
}
//-------- End of function RebelArray::stop_attack_firm ---------//


//### begin alex 20/8 ###//
//--------- Begin of function RebelArray::stop_attack_nation ---------//
void RebelArray::stop_attack_nation(short nationRecno)
{
	Rebel *rebelPtr;

	for(int i=size(); i>0; i--)
	{
		rebelPtr = (Rebel*) get_ptr(i);

		if(rebelPtr)
			rebelPtr->reset_hostile_nation(nationRecno);
	}
}
//-------- End of function RebelArray::stop_attack_nation ---------//
//#### end alex 20/8 ####//

//--------- Begin of function RebelArray::next_day ---------//
//
void RebelArray::next_day()
{
	int   i;
	Rebel *rebelPtr;

	for( i=size() ; i>0 ; i-- )
	{
		rebelPtr = (Rebel*) get_ptr(i);

		if( rebelPtr )
			rebelPtr->next_day();
	}
}
//----------- End of function RebelArray::next_day ---------//


//--------- Begin of function RebelArray::drop_rebel_identity -------//
//
// 4 fates for a rebel unit:
//
// - the rebel gets killed
// - the rebel group settles down in a town
// - the rebel surrender a nation
// - the rebel group merges with another rebel group
//
// This function handle the fate that the unit gets killed.
//
void RebelArray::drop_rebel_identity(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->unit_mode != UNIT_MODE_REBEL );

	err_when( rebel_array.is_deleted(unitPtr->unit_mode_para) );

	//---- decrease the unit count of the rebel group ----//

	int	 rebelRecno = unitPtr->unit_mode_para;
	Rebel* rebelPtr   = rebel_array[rebelRecno];

	rebelPtr->mobile_rebel_count--;

	unitPtr->set_mode(0);		// drop its rebel identity 

	//----- if all rebels are dead and the rebel doesn't occupy a town, del the rebel group ----//

	if( rebelPtr->mobile_rebel_count==0 && rebelPtr->town_recno==0 )
	{
		del_rebel(rebelRecno);
		return;
	}

	//----- when the rebel leader is killed -----//

	if( rebelPtr->leader_unit_recno == unitRecno )
		rebelPtr->process_leader_quit();
}
//----------- End of function RebelArray::drop_rebel_identity --------//


//--------- Begin of function Rebel::process_leader_quit -------//
//
void Rebel::process_leader_quit()
{
	//-------------------------------------------//
	//
	// When the rebel leader gets killed, a new rebel unit
	// in the group is elected as the new leader.
	//
	// Some the rebel units leave the rebel group to:
	// surrender to a nation (moving into a town as town people)
	//
	//-------------------------------------------//

	//----- select a new unit as the leader ------//

	leader_unit_recno = 0;

	if( mobile_rebel_count>0 )		// it must at least be 2: the dying leader + one rebel soldier
	{
		select_new_leader();
	}
	else
	{
		err_when( !town_recno );		// if mobile_label_count is 0, the rebel group must have a town

		// if the rebel has a town, leader_unit_recno can be 0
	}

	//------ some the rebel units leave the rebel group -----//

	if( mobile_rebel_count>1 )
	{
		//---- surrender to a nation ----//

		int 	  maxReputation=0, bestNationRecno=0;
		Nation* nationPtr;

		int i;
		for( i=nation_array.size() ; i>0 ; i-- )
		{
			if( nation_array.is_deleted(i) )
				continue;

			nationPtr = nation_array[i];

			if( nationPtr->reputation > maxReputation )
			{
				maxReputation   = (int) nationPtr->reputation;
				bestNationRecno = i;
			}
		}

		if( !bestNationRecno )		// no nation has a positive reputation
			return;

		//------- process the rebel units -------//

		Unit* unitPtr;

		for( i=unit_array.size() ; i>0 ; i-- )
		{
			if( unit_array.is_deleted(i) )
				continue;

			unitPtr = unit_array[i];

			if( unitPtr->unit_mode == UNIT_MODE_REBEL &&
				 unitPtr->unit_mode_para == leader_unit_recno )
			{
				unitPtr->set_mode(0);
				unitPtr->change_nation(bestNationRecno);
			}
		}
	}
}
//-------- End of function Rebel::process_leader_quit --------//


//--------- Begin of function Rebel::select_new_leader -------//
//
int Rebel::select_new_leader()
{
	if( mobile_rebel_count==0 )
		return 0;

	Unit* unitPtr;

	int i;
	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->unit_mode==UNIT_MODE_REBEL &&
			 unitPtr->unit_mode_para == rebel_recno )
		{
			err_when( unitPtr->hit_points<=0 );

			unitPtr->set_rank(RANK_GENERAL);
			leader_unit_recno = i;
			break;
		}
	}

	if( !leader_unit_recno )
		return 0;

	//--- update the leader_unit_renco of all units in this rebel group ---//

	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->unit_mode==UNIT_MODE_REBEL &&
			 unitPtr->unit_mode_para == rebel_recno )
		{
			unitPtr->leader_unit_recno = leader_unit_recno;
		}
	}

	return 0;
}
//-------- End of function Rebel::select_new_leader --------//


//-------- Begin of function RebelArray::settle_town -------//
//
// A unit settle_towns from its rebel group.
//
// <int> unitRecno - recno of the unit going to settle in the town
// <int> townRecno - recno of the town this unit settles
//
void RebelArray::settle_town(int unitRecno, int townRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->unit_mode != UNIT_MODE_REBEL );

	err_when( rebel_array.is_deleted(unitPtr->unit_mode_para) );

	//---- decrease the unit count of the rebel group ----//

	int	 rebelRecno = unitPtr->unit_mode_para;
	Rebel* rebelPtr   = rebel_array[rebelRecno];

	rebelPtr->mobile_rebel_count--;

	//--------- settle in a town ----------//

//	err_when( rebelPtr->town_recno && rebelPtr->town_recno != townRecno );		// one rebel group only allow settle in a town, error when trying to settle in more than one town

	Town* townPtr = town_array[townRecno];

	if( rebelPtr->town_recno==0 && townPtr->rebel_recno==0 )
	{
		rebelPtr->town_recno = townRecno;
		townPtr->rebel_recno = rebelRecno;
	}
}
//----------- End of function RebelArray::settle_town --------//


//--------- Begin of function Rebel::Rebel --------//
//
Rebel::Rebel()
{
	memset( this, 0, sizeof(Rebel) );
	hostile_nation_bits = 0;
}
//----------- End of function Rebel::Rebel ---------//


//--------- Begin of function Rebel::~Rebel --------//
//
Rebel::~Rebel()
{
	Unit* unitPtr;

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->unit_mode == UNIT_MODE_REBEL &&
			 unitPtr->unit_mode_para == rebel_recno )
		{
			unitPtr->set_mode(0);
		}
	}
}
//----------- End of function Rebel::~Rebel ---------//


//--------- Begin of function Rebel::join ---------//
//
// A unit joins this rebel group as a rebel member.
// The rebel leader exists since the creation of this rebel group.
//
void Rebel::join(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->nation_recno );
	err_when( unitPtr->spy_recno    );

	unitPtr->set_mode( UNIT_MODE_REBEL, rebel_recno );

	mobile_rebel_count++;
}
//----------- End of function Rebel::join ---------//


//--------- Begin of function Rebel::think_cur_action ---------//
//
void Rebel::think_cur_action()
{
	//------ if the rebel is attacking a town -------//

	if( action_mode == REBEL_ATTACK_TOWN )
	{
		//----- the town has already been captured -----//

		if( town_array.is_deleted(action_para) || town_array[action_para]->nation_recno == 0 )
		{
			//--- stop all units that are still attacking the town ---//

			stop_all_rebel_unit();
			set_action(REBEL_IDLE);
		}
	}

	//----- if the rebel should be attacking a firm -----//

	if( action_mode == REBEL_ATTACK_FIRM )
	{
		if( firm_array.is_deleted(action_para) || firm_array[action_para]->nation_recno == 0 )
		{
			//--- stop all units that are still attacking the firm ---//

			stop_all_rebel_unit();

			set_action(REBEL_IDLE);
		}
	}
}
//----------- End of function Rebel::think_cur_action ---------//


//--------- Begin of function Rebel::next_day ---------//
//
void Rebel::next_day()
{
	//---- if the rebels has a town and this rebel is a defender from a town ----//

	if( town_recno )
	{
		//--- check if the rebel town is destroyed ----//

		if( town_array.is_deleted(town_recno) )
		{
			town_recno = 0;

			//--- if the town has been destroyed, the rebel group becomes mobile again, and a new leader needs to be selected.

			if( mobile_rebel_count>0 && leader_unit_recno==0 )
			{
				select_new_leader();
			}
		}

		err_when( town_recno && town_array[town_recno]->rebel_recno != rebel_recno );

		return;		// don't think new action as this rebel defender need to go back to its town.
	}

	//------- no rebels left in the group --------//

	if( town_recno==0 && mobile_rebel_count==0 )
	{
		rebel_array.del_rebel(rebel_recno);
		return;
	}

	//---------------------------------------------//

	err_when( mobile_rebel_count==0 && town_recno==0 );		// if both are zero, this rebel group should have been deleted.

	if( mobile_rebel_count > 0 ) 		// if there are mobile rebel units on the map
	{
		if( action_mode==REBEL_IDLE )
			think_new_action();				// think about a new action
		else
			think_cur_action();				// think if there should be any changes to the current action
	}
	else			// if there are no mobile rebel units
	{
		if( town_recno > 0 )
		{
			if( info.game_date%30 == rebel_recno%30 )		// if the rebel has a town
				think_town_action();
		}
	}
}
//----------- End of function Rebel::next_day ---------//


//--------- Begin of function Rebel::think_new_action ---------//
//
void Rebel::think_new_action()
{
	if(unit_array.is_deleted(leader_unit_recno))
		return;

	int rc;

	switch( misc.random(4) )
	{
		case 0:
			rc = think_settle_new();
			break;

		case 1:
			rc = think_settle_to();
			break;

		case 2:
			rc = think_capture_attack_town();
			break;

		case 3:
			rc = think_attack_firm();
			break;
	}

	if( rc )
		execute_new_action();
}
//----------- End of function Rebel::think_new_action ---------//


//--------- Begin of function Rebel::think_settle_new ---------//
//
int Rebel::think_settle_new()
{
	//------- get the leader unit's info -----------//

	Unit* leaderUnit = unit_array[leader_unit_recno];

	int   leaderXLoc=leaderUnit->cur_x_loc(), leaderYLoc=leaderUnit->cur_y_loc();

	//----------------------------------------------//

	int xLoc2 = leaderXLoc + STD_TOWN_LOC_WIDTH  - 1;
	int yLoc2 = leaderYLoc + STD_TOWN_LOC_HEIGHT - 1;

	if( xLoc2 >= MAX_WORLD_X_LOC )
	{
		xLoc2   = MAX_WORLD_X_LOC - 1;
		leaderXLoc = xLoc2 - STD_TOWN_LOC_WIDTH + 1;
	}

	if( yLoc2 >= MAX_WORLD_Y_LOC )
	{
		yLoc2   = MAX_WORLD_Y_LOC - 1;
		leaderYLoc = yLoc2 - STD_TOWN_LOC_HEIGHT + 1;
	}

	int regionId = world.get_region_id( leaderXLoc, leaderYLoc );

	if( world.locate_space( leaderXLoc, leaderYLoc, xLoc2, yLoc2, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, UNIT_LAND, regionId, 1 ) )
	{
		action_mode  = REBEL_SETTLE_NEW;
		action_para  = leaderXLoc;
		action_para2 = leaderYLoc;

		return 1;
	}

	return 0;
}
//----------- End of function Rebel::think_settle_new ---------//


//--------- Begin of function Rebel::think_settle_to ---------//
//
// Think about settling to an existing rebel town.
//
int Rebel::think_settle_to()
{
	int   i, townRecno;
	Town* townPtr;
	Unit* leaderUnit = unit_array[leader_unit_recno];
	int   curRegionId = world.get_region_id(leaderUnit->cur_x_loc(), leaderUnit->cur_y_loc());

	townRecno = misc.random(town_array.size())+1;

	for( i=1 ; i<=town_array.size() ; i++ )
	{
		if( ++townRecno > town_array.size() )
			townRecno = 1;

		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( !townPtr->rebel_recno )
			continue;

		if( world.get_region_id(townPtr->loc_x1, townPtr->loc_y1) != curRegionId )
			continue;

		if( leaderUnit->race_id == townPtr->majority_race() )
		{
			action_mode  = REBEL_SETTLE_TO;
			action_para  = townPtr->loc_x1;
			action_para2 = townPtr->loc_y1;
			return 1;
		}
	}

	return 0;
}
//----------- End of function Rebel::think_settle_to ---------//


//--------- Begin of function Rebel::think_capture_attack_town ---------//
//
int Rebel::think_capture_attack_town()
{
	//------- get the leader unit's info -----------//

	Unit* leaderUnit = unit_array[leader_unit_recno];
	int   leaderXLoc = leaderUnit->cur_x_loc(), leaderYLoc=leaderUnit->cur_y_loc();
	int   curRegionId = world.get_region_id(leaderUnit->cur_x_loc(), leaderUnit->cur_y_loc());

	//----------------------------------------------//

	int actionMode = REBEL_ATTACK_TOWN;

	int 	townRecno, bestTownRecno=0;
	int	townDistance, closestTownDistance=0x7FFF;
	Town* townPtr;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		townRecno = misc.random(town_array.size())+1;

		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( !is_hostile_nation(townPtr->nation_recno) )
			continue;

		if( world.get_region_id(townPtr->loc_x1, townPtr->loc_y1) != curRegionId )
			continue;

		townDistance = misc.points_distance( leaderXLoc, leaderYLoc,
							townPtr->center_x, townPtr->center_y );

		if( townDistance < closestTownDistance )
		{
			closestTownDistance = townDistance;
			bestTownRecno		  = townRecno;
		}
	}

	if( bestTownRecno )
	{
		action_mode = actionMode;
		action_para = bestTownRecno;		// attack this town
		return 1;
	}

	return 0;
}
//----------- End of function Rebel::think_capture_attack_town ---------//


//--------- Begin of function Rebel::think_attack_firm ---------//
//
int Rebel::think_attack_firm()
{
	//------- get the leader unit's info -----------//

	Unit* leaderUnit = unit_array[leader_unit_recno];
	int   leaderXLoc=leaderUnit->cur_x_loc(), leaderYLoc=leaderUnit->cur_y_loc();
	int   curRegionId = world.get_region_id(leaderUnit->cur_x_loc(), leaderUnit->cur_y_loc());

	//----------------------------------------------//

	int 	firmRecno, bestFirmRecno=0;
	int	firmDistance, closestFirmDistance=0x7FFF;
	Firm* firmPtr;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		firmRecno = misc.random(firm_array.size())+1;

		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( !is_hostile_nation(firmPtr->nation_recno) )
			continue;

		if( world.get_region_id(firmPtr->loc_x1, firmPtr->loc_y1) != curRegionId )
			continue;

		firmDistance = misc.points_distance( leaderXLoc, leaderYLoc,
							firmPtr->center_x, firmPtr->center_y );

		if( firmDistance < closestFirmDistance )
		{
			closestFirmDistance = firmDistance;
			bestFirmRecno		  = firmRecno;
		}
	}

	if( bestFirmRecno )
	{
		action_mode = REBEL_ATTACK_FIRM;
		action_para = bestFirmRecno;		// attack this town
		return 1;
	}

	return 0;
}
//----------- End of function Rebel::think_attack_firm ---------//


//--------- Begin of function Rebel::execute_new_action -------//
//
// Called by think_new_action(), execute the action immediately right
// after a new action has been decided.
//
void Rebel::execute_new_action()
{
	//----- create an recno array of the rebel units ----//

	short* rebelRecnoArray = (short*) mem_add( mobile_rebel_count * sizeof(short) );
	Unit*  unitPtr;
	int	 rebelCount=0;

	#ifdef DEBUG
		int dieUnit = 0;
	#endif

	for( int i=unit_array.size() ; i>=1 ; i-- )
	{
		if( unit_array.is_deleted(i) )
		{
			#ifdef DEBUG
				if(!unit_array.SpriteArray::is_deleted(i))
					dieUnit++;
			#endif

			continue;
		}

		unitPtr = unit_array[i];

		if( unitPtr->unit_mode == UNIT_MODE_REBEL &&
			 unitPtr->unit_mode_para == rebel_recno )
		{
			err_when( rebelCount >= mobile_rebel_count );

			rebelRecnoArray[rebelCount++] = i;
		}
	}

#ifdef DEBUG
	if(rebelCount+dieUnit != mobile_rebel_count)
		int debug = 0;
#endif

	if( !rebelCount )
	{
		mem_del( rebelRecnoArray );
		return; // all rebel units are dead
	}

	//-------- execute the new action now --------//

	Town* townPtr;
	Firm* firmPtr;

	switch( action_mode )
	{
		case REBEL_ATTACK_TOWN:
			err_when( town_array.is_deleted(action_para) || town_array[action_para]->nation_recno==0 );		// these should have been checked before calling this function

			townPtr = town_array[action_para];
			// ##### patch begin Gilbert 5/8 ######//
			unit_array.attack( townPtr->loc_x1, townPtr->loc_y1, 0, rebelRecnoArray, rebelCount, COMMAND_AI, 0 );
			// ##### patch end Gilbert 5/8 ######//
			break;

		case REBEL_ATTACK_FIRM:
			err_when( firm_array.is_deleted(action_para) || firm_array[action_para]->nation_recno==0 );		// these should have been checked before calling this function

			firmPtr = firm_array[action_para];
			// ##### patch begin Gilbert 5/8 ######//
			unit_array.attack( firmPtr->loc_x1, firmPtr->loc_y1, 0, rebelRecnoArray, rebelCount, COMMAND_AI, 0 );
			// ##### patch end Gilbert 5/8 ######//
			break;

		case REBEL_SETTLE_NEW:
			unit_array.settle( action_para, action_para2, 0, 1, rebelRecnoArray, rebelCount );		// action_para & action_para2 is the destination location
			break;

		case REBEL_SETTLE_TO:
			unit_array.assign( action_para, action_para2, 0, 1, rebelRecnoArray, rebelCount);
			break;
	}

	mem_del( rebelRecnoArray );
}
//-------- End of function Rebel::execute_new_action ---------//


//--------- Begin of function Rebel::think_town_action ---------//
//
// Call this function to think about the new actions of this
// rebel group when all members of the rebel has settled down
// into a town.
//
void Rebel::think_town_action()
{
	if( town_recno==0 || mobile_rebel_count>0 )	// only when all rebel units has settled in the town
		return;

	//----- neutralize to an independent town -----//

	if( misc.random(10)==0 )
	{
		turn_indepedent();
	}

	//---------- form a nation ---------//

	else if( misc.random(10)==0 )
	{
		if( town_array[town_recno]->population >= 20 &&
			 nation_array.can_form_new_ai_nation() )
		{
			town_array[town_recno]->form_new_nation();
		}
	}
}
//----------- End of function Rebel::think_town_action ---------//


//--------- Begin of function Rebel::turn_indepedent ---------//
//
// The rebel town turns into an indepedent town.
//
void Rebel::turn_indepedent()
{
	err_when( town_recno==0 || mobile_rebel_count>0 );				// only when all rebel units has settled in the town

	town_array[town_recno]->rebel_recno = 0;

	//------ remove this rebel group from rebel_array ------//

	rebel_array.del_rebel(rebel_recno);
}
//----------- End of function Rebel::turn_indepedent ---------//


//--------- Begin of function Rebel::stop_all_rebel_unit ---------//
//
void Rebel::stop_all_rebel_unit()
{
	Unit* unitPtr;

	for( int i=unit_array.size() ; i>=1 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->unit_mode == UNIT_MODE_REBEL &&
			 unitPtr->unit_mode_para == rebel_recno )
		{
			unitPtr->stop();
		}
	}
}
//----------- End of function Rebel::stop_all_rebel_unit ---------//


//--------- Begin of function Rebel::town_being_attacked ---------//
//
// This rebel group's town is being attacked.
//
void Rebel::town_being_attacked(int attackerUnitRecno)
{
	err_when( !town_recno );

	//----------------------------------------------//
	//
	// Set the hostile_nation_recno. So that if the rebel
	// town is destroyed and there are rebel units left,
	// they can form a rebel group again and battle with
	// the attacking nation.
	//
	//----------------------------------------------//

	set_hostile_nation(unit_array[attackerUnitRecno]->nation_recno);
}
//----------- End of function Rebel::town_being_attacked ---------//


//--------- Begin of function Rebel::set_hostile_nation ---------//
void Rebel::set_hostile_nation(short nationRecno)
{
	if(nationRecno==0)
		return;

	err_when(nationRecno>7); // only 8 bits
	hostile_nation_bits |= (0x1 << nationRecno);
}
//----------- End of function Rebel::set_hostile_nation ---------//


//--------- Begin of function Rebel::reset_hostile_nation ---------//
void Rebel::reset_hostile_nation(short nationRecno)
{
	if(nationRecno==0)
		return;

	err_when(nationRecno>7); // only 8 bits
	hostile_nation_bits &= ~(0x1 << nationRecno);
}
//----------- End of function Rebel::reset_hostile_nation ---------//


//--------- Begin of function Rebel::is_hostile_nation ---------//
// return 1 for hostile nation
// return 0 otherwise
//
int Rebel::is_hostile_nation(short nationRecno)
{
	if(nationRecno==0)
		return 0;

	err_when(nationRecno>7); // only 8 bits
	return (hostile_nation_bits & (0x1 << nationRecno));
}
//----------- End of function Rebel::is_hostile_nation ---------//


//--------- Begin of function Unit::process_rebel ---------//
//
// Note: this is a Unit member function, not a Rebel member function,
// it is called by Unit::process_idle();
//
void Unit::process_rebel()
{
	Rebel* rebelPtr = rebel_array[unit_mode_para];

	switch( rebelPtr->action_mode )
	{
		case REBEL_ATTACK_TOWN:
			if( town_array.is_deleted(rebelPtr->action_para) )	// if the town has been destroyed.
				rebelPtr->set_action(REBEL_IDLE);
			else
			{
				Town* townPtr = town_array[rebelPtr->action_para];
				attack_town( townPtr->loc_x1, townPtr->loc_y1 );
			}
			break;

		case REBEL_SETTLE_NEW:
			if( !world.can_build_town(rebelPtr->action_para, rebelPtr->action_para2, sprite_recno) )
			{
				Location* locPtr = world.get_loc(rebelPtr->action_para, rebelPtr->action_para2);

				if( locPtr->is_town() &&
					 town_array[locPtr->town_recno()]->rebel_recno == rebelPtr->rebel_recno )
				{
					rebelPtr->action_mode = REBEL_SETTLE_TO;
				}
				else
				{
					rebelPtr->action_mode = REBEL_IDLE;
				}
			}
			else
			{
				settle( rebelPtr->action_para, rebelPtr->action_para2 );
			}
			break;

		case REBEL_SETTLE_TO:
			assign( rebelPtr->action_para, rebelPtr->action_para2 );
			break;
	}
}
//----------- End of function Unit::process_rebel -----------//


#ifdef DEBUG

//------- Begin of function RebelArray::operator[] -----//

Rebel* RebelArray::operator[](int recNo)
{
	Rebel* rebelPtr = (Rebel*) get_ptr(recNo);

	if( !rebelPtr )
		err.run( "RebelArray[] is deleted" );

	return rebelPtr;
}
//--------- End of function RebelArray::operator[] ----//

#endif
