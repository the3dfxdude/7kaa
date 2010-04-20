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

//Filename   : OU_GOD.CPP
//Description: God Unit

#include <OSYS.h>
#include <OGAME.h>
#include <OBUTT3D.h>
#include <OGODRES.h>
#include <OF_BASE.h>
#include <ORACERES.h>
#include <OPOWER.h>
#include <OU_MARI.h>
#include <OU_GOD.h>
#include <OWEATHER.h>
#include <OTORNADO.h>

//--------- Define static vars ----------//

static Button3D button_cast, button_cast2;

//--------- Begin of function UnitGod::init_derived --------//
void UnitGod::init_derived()
{
	// ##### patch begin Gilbert 22/1 ######//
	cast_power_type = 0;
	// ##### end begin Gilbert 22/1 ######//
	if(unit_id==UNIT_PERSIAN_HEALER || unit_id==UNIT_VIKING_GOD ||
		unit_id==UNIT_KUKULCAN || unit_id==UNIT_JAPANESE_GOD)
		can_attack_flag = 0; // unable to attack

#ifdef AMPLUS
	if(unit_id==UNIT_EGYPTIAN_GOD || unit_id==UNIT_INDIAN_GOD ||
		unit_id==UNIT_ZULU_GOD)
		can_attack_flag = 0; // unable to attack
#endif
}
//-------- End of function UnitGod::init_derived -------//


//------- Begin of function UnitGod::pre_process -------//
//
void UnitGod::pre_process()
{
	Unit::pre_process();

	if( game.game_mode == GAME_TEST )
		return;

	//---- set force_move_flag to 1 if the god does not have the ability to attack ----//

	if( god_id != GOD_CHINESE && god_id != GOD_NORMAN )		// only Chinese and Norman dragon can attack
		force_move_flag = 1;

	//-*********** simulate aat ************-//
	#ifdef DEBUG
		if(debug_sim_game_type)
			return;
	#endif
	//-*********** simulate aat ************-//

	//--- if the seat of power supporting this unit is destroyed, this unit dies ---//

	if( firm_array.is_deleted(base_firm_recno) )
	{
		hit_points=(float)0;
		set_die();
		return;
	}

	//---- this unit consume pray points as it exists ----//

	FirmBase* firmBase = (FirmBase*) firm_array[base_firm_recno];

	err_when( firmBase->firm_id != FIRM_BASE );

	firmBase->pray_points -= (float) god_res[god_id]->exist_pray_points / 200;

	if( firmBase->pray_points < 0 )
		firmBase->pray_points = (float) 0;

	//--------- update hit points --------//

	hit_points = (short) firmBase->pray_points;

	if( hit_points == 0 )
		set_die();
}
//-------- End of function UnitGod::pre_process -------//


//--------- Begin of function UnitGod::process_attack --------//

int UnitGod::process_attack()
{
	if( !Sprite::process_attack() )		// return 1 if the unit just finished its current attack
		return 0;

	if( game.game_mode == GAME_TEST )
		return 1;

	//-*********** simulate aat ************-//
	#ifdef DEBUG
		if(debug_sim_game_type)
			return 1;
	#endif
	//-*********** simulate aat ************-//

	consume_power_pray_points();

	return 1;
}
//---------- End of function UnitGod::process_attack ----------//


//--------- Begin of function UnitGod::disp_info ---------//
//
void UnitGod::disp_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);
	disp_unit_profile( INFO_Y1+54, refreshFlag );

	if( !is_own() )
		return;

	if( game.game_mode == GAME_TEST )
		return;

	if( god_res[god_id]->can_cast_power )
	{
		//-------- get the button name --------//

		const char* buttonName;

		switch(god_id)
		{
			case GOD_PERSIAN:
				buttonName = "GODHEAL";
				break;

			case GOD_JAPANESE:
				buttonName = "GODMIND";
				break;

			case GOD_MAYA:
				buttonName = "GODINCCL";
				break;

			case GOD_VIKING:
				buttonName = "GODRAIN";
				break;

#ifdef AMPLUS
			case GOD_EGYPTIAN:
				buttonName = "GODEGYPT";
				break;

			case GOD_INDIAN:
				buttonName = "GODMUGL";
				break;

			case GOD_ZULU:
				buttonName = "GODZULU";
				break;
#endif

			default:
				err_here();
		}

		//----------- create the cast button -----------//

		button_cast.paint(INFO_X1, INFO_Y1+101, 'A', buttonName );

		if( hit_points >= god_res[god_id]->power_pray_points )
			button_cast.enable();
		else
			button_cast.disable();

		//------ The viking gods have two types of power ------//

		if( god_id == GOD_VIKING )
		{
			button_cast2.paint(INFO_X1+BUTTON_ACTION_WIDTH, INFO_Y1+101, 'A', "GODTORNA" );

			if( hit_points >= god_res[god_id]->power_pray_points )
				button_cast2.enable();
			else
				button_cast2.disable();
		}
	}
}
//---------- End of function UnitGod::disp_info ----------//


//--------- Begin of function UnitGod::detect_info ---------//
//
void UnitGod::detect_info()
{
	if( detect_basic_info() )
		return;

	if( detect_unit_profile() )
		return;

	if( !is_own() )
		return;

	if( game.game_mode == GAME_TEST )
		return;

	if( god_res[god_id]->can_cast_power )
	{
		// ###### begin Gilbert 14/10 ######//
		int rc=0;
		char castPowerType = 0;

		if( button_cast.detect() )
		{
			// cast_power_type = 1;
			castPowerType = 1;
			rc = 1;
		}

		if( button_cast2.detect() )
		{
			castPowerType = 2;
			//cast_power_type = 2;
			//cast_origin_x = cur_x_loc();
			//cast_origin_y = cur_y_loc();
			rc = 1;
		}

		//----------------------------------------//

		if( rc && castPowerType)
		{
			if( god_id == GOD_VIKING && castPowerType == 1 )	// summon rain, summon immediately, no need to select target
				go_cast_power(next_x_loc(), next_y_loc(), castPowerType, COMMAND_PLAYER);
			else
				power.issue_command(COMMAND_GOD_CAST_POWER, sprite_recno, castPowerType);
		}
		// ###### end Gilbert 14/10 ######//
	}
}
//---------- End of function UnitGod::detect_info ----------//


//--------- Begin of function UnitGod::cast_power ---------//
//
// <int> castXLoc, castYLoc - the location which the power should
//										be casted on.
//
void UnitGod::cast_power(int xLoc, int yLoc)
{
	err_when( !god_res[god_id]->can_cast_power );

	//------- consumer pray points --------//

	// consume_power_pray_points();		// let process_attack to consume

	//---- viking god does not need a range for casting power ----//

	if( god_id == GOD_VIKING )
	{
		if( cast_power_type == 1 )
			viking_summon_rain();
		else
			viking_summon_tornado();
		return;
	}

	//------ cast power on the selected area ------//

	GodInfo* godInfo = god_res[god_id];

	int xLoc1 = xLoc - godInfo->cast_power_range + 1;
	int yLoc1 = yLoc - godInfo->cast_power_range + 1;
	int xLoc2 = xLoc + godInfo->cast_power_range - 1;
	int yLoc2 = yLoc + godInfo->cast_power_range - 1;

	int t;
	int centerY = (yLoc1+yLoc2) / 2;
	Location* locPtr;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		t=abs(yLoc-centerY)/2;
		locPtr = world.get_loc(xLoc1+t, yLoc);

		for( xLoc=xLoc1+t ; xLoc<=xLoc2-t ; xLoc++, locPtr++ )
		{
			if( xLoc>=0 && xLoc<MAX_WORLD_X_LOC &&
				 yLoc>=0 && yLoc<MAX_WORLD_Y_LOC )
			{
				cast_on_loc(xLoc, yLoc);
			}
		}
	}
}
//---------- End of function UnitGod::cast_power ----------//


//--------- Begin of function UnitGod::viking_summon_rain ---------//
//
void UnitGod::viking_summon_rain()
{
	magic_weather.cast_rain(10, 8);         // 10 days, rain scale 8
	magic_weather.cast_lightning(7 );               // 7 days
}
//---------- End of function UnitGod::viking_summon_rain ----------//


//--------- Begin of function UnitGod::viking_summon_tornado ---------//
//
void UnitGod::viking_summon_tornado()
{
	// ######## begin Gilbert 14/10 ########//
	short xLoc = next_x_loc();
	short yLoc = next_y_loc();
	char dir = final_dir % 8;
	// ######## end Gilbert 14/10 ########//

	// put a tornado one location ahead
	if( dir == 0 || dir == 1 || dir == 7 )
		if( yLoc > 0 )
			yLoc--;
	if( dir >= 1 && dir <= 3 )
		if( xLoc < MAX_WORLD_X_LOC-1 )
			xLoc++;
	if( dir >= 3 && dir <= 5 )
		if( yLoc < MAX_WORLD_Y_LOC-1)
			yLoc++;
	if( dir >= 5 && dir <= 7 )
		if( xLoc > 0 )
			xLoc--;

	tornado_array.add_tornado( xLoc, yLoc, 600 );
	magic_weather.cast_wind(10, 1, dir * 45 );              // 10 days
}
//---------- End of function UnitGod::viking_summon_tornado ----------//


//--------- Begin of function UnitGod::cast_on_loc ---------//
//
void UnitGod::cast_on_loc(int castXLoc, int castYLoc)
{
	Location* locPtr = world.get_loc( castXLoc, castYLoc );

	//--- if there is any unit on the location ---//

	if( locPtr->has_unit(UNIT_LAND) )
	{
		cast_on_unit( locPtr->unit_recno(UNIT_LAND), 1 );
	}
	else if( locPtr->has_unit(UNIT_SEA) )
	{
		Unit* unitPtr = unit_array[ locPtr->unit_recno(UNIT_SEA) ];

		//-- only heal human units belonging to our nation in ships --//

		if( unitPtr->nation_recno == nation_recno &&
			 unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_SHIP )
		{
			UnitMarine* unitMarine = (UnitMarine*) unitPtr;

			for( int i=0 ; i<unitMarine->unit_count ; i++ )
			{
				int divider = 4;		// the size of a ship is 4 locations (2x2)

				cast_on_unit( unitMarine->unit_recno_array[i], divider );		// the effects are weaken on ship units, only 50% of the original effects
			}
		}
	}

	//--------- on firms ---------//

	else if( locPtr->is_firm() )
	{
		Firm* firmPtr = firm_array[ locPtr->firm_recno() ];
		int	divider = (firmPtr->loc_x2-firmPtr->loc_x1+1) * (firmPtr->loc_y2-firmPtr->loc_y1+1);
#ifdef AMPLUS
		if( god_id == GOD_ZULU )
			divider = 1;		// range of zulu god is 1, no need to divide
#endif

		if( firmPtr->overseer_recno )
		{
			cast_on_unit( firmPtr->overseer_recno, divider );
		}

		if( firmPtr->worker_array && firm_res[firmPtr->firm_id]->live_in_town==0 )
		{
			Worker* workerPtr = firmPtr->worker_array;

			for( int i=0 ; i<firmPtr->worker_count ; i++, workerPtr++ )
			{
				cast_on_worker(workerPtr, firmPtr->nation_recno, divider);
			}
		}
	}

	//--------- on towns ----------//

	else if( locPtr->is_town() )
	{
		Town* townPtr = town_array[ locPtr->town_recno() ];

		if( god_id == GOD_JAPANESE && townPtr->nation_recno != nation_recno)
		{
			int divider = STD_TOWN_LOC_WIDTH * STD_TOWN_LOC_HEIGHT;

			for( int i=0 ; i<MAX_RACE ; i++ )
			{
				if( townPtr->race_pop_array[i]==0 )
					continue;

				float changePoints = (float)7 + m.random(8);		// decrease 7 to 15 loyalty points instantly

				if( townPtr->nation_recno )
					townPtr->change_loyalty(i+1, -changePoints/divider);
				else
					townPtr->change_resistance(i+1, nation_recno, -changePoints/divider);
			}
		}
#ifdef AMPLUS
		else if( god_id == GOD_EGYPTIAN && townPtr->nation_recno == nation_recno)
		{
			int headCount;
			int raceId;

			for( headCount = 5; headCount > 0 && townPtr->population < MAX_TOWN_GROWTH_POPULATION
				&& (raceId = townPtr->pick_random_race(1,1)); --headCount )
			{
				townPtr->inc_pop(raceId, 0, (int)townPtr->race_loyalty_array[raceId-1]);
			}
		}
#endif
	}
}
//---------- End of function UnitGod::cast_on_loc ----------//


//--------- Begin of function UnitGod::cast_on_unit ---------//
//
// <int> unitRecno - recno of the unit to cast on
// <int> divider   - divide the amount of effects by this number
//
void UnitGod::cast_on_unit(int unitRecno, int divider)
{
	switch(god_id)
	{
		case GOD_PERSIAN:
			persian_cast_power( unitRecno, divider );
			break;

		case GOD_JAPANESE:
			japanese_cast_power( unitRecno, divider );
			break;

		case GOD_MAYA:
			maya_cast_power( unitRecno, divider );
			break;

#ifdef AMPLUS
		case GOD_EGYPTIAN:
			egyptian_cast_power( unitRecno, divider);
			break;

		case GOD_INDIAN:
			indian_cast_power( unitRecno, divider);
			break;

		case GOD_ZULU:
			zulu_cast_power( unitRecno, divider);
			break;
#endif

		default:
			err_here();
	}
}
//---------- End of function UnitGod::cast_on_unit ----------//


//--------- Begin of function UnitGod::cast_on_worker ---------//
//
// <Worker*> workerPtr - pointer to the worker to be affected by the effect.
//
void UnitGod::cast_on_worker(Worker* workerPtr, int nationRecno, int divider)
{
	switch(god_id)
	{
		case GOD_PERSIAN:
			persian_cast_power( workerPtr, nationRecno, divider );
			break;

		case GOD_JAPANESE:
			japanese_cast_power( workerPtr, nationRecno, divider );
			break;

		case GOD_MAYA:
			maya_cast_power( workerPtr, nationRecno, divider );
			break;

#ifdef AMPLUS
		case GOD_EGYPTIAN:
			egyptian_cast_power(workerPtr, nationRecno, divider);
			break;

		case GOD_INDIAN:
			indian_cast_power(workerPtr, nationRecno, divider);
			break;
	
		case GOD_ZULU:
			zulu_cast_power(workerPtr, nationRecno, divider);
			break;
#endif

		default:
			err_here();
	}
}
//---------- End of function UnitGod::cast_on_worker ----------//


//--------- Begin of function UnitGod::persian_cast_power ---------//
//
void UnitGod::persian_cast_power(int unitRecno, int divider)
{
	Unit* unitPtr = unit_array[unitRecno];

	//-- only heal human units belonging to our nation --//

	if( unitPtr->nation_recno == nation_recno && unitPtr->race_id > 0 )
	{
		float changePoints = (float) unitPtr->max_hit_points / (6+m.random(4));	 // divided by (6 to 9)

		changePoints = MAX( changePoints, 10 );

		unitPtr->change_hit_points( changePoints/divider );
	}
}
//---------- End of function UnitGod::persian_cast_power ----------//


//--------- Begin of function UnitGod::japanese_cast_power ---------//
//
void UnitGod::japanese_cast_power(int unitRecno, int divider)
{
	Unit* unitPtr = unit_array[unitRecno];

	//-- only cast on enemy units -----//

	if( unitPtr->nation_recno != nation_recno && unitPtr->race_id > 0 )
	{
		int changePoints = 7 + m.random(8);		// decrease 7 to 15 loyalty points instantly

		unitPtr->change_loyalty( -MAX(1, changePoints/divider) );
	}
}
//---------- End of function UnitGod::japanese_cast_power ----------//


//--------- Begin of function UnitGod::maya_cast_power ---------//
//
void UnitGod::maya_cast_power(int unitRecno, int divider)
{
	Unit* unitPtr = unit_array[unitRecno];

	//-- only cast on mayan units belonging to our nation --//

	if( unitPtr->nation_recno == nation_recno && unitPtr->race_id == RACE_MAYA )
	{
		int changePoints = 15 + m.random(10);		// add 15 to 25 points to its combat level instantly

		int newCombatLevel = unitPtr->skill.combat_level + changePoints/divider;

		if( newCombatLevel > 100 )
			newCombatLevel = 100;

		float oldHitPoints = unitPtr->hit_points;

		unitPtr->set_combat_level(newCombatLevel);

		unitPtr->hit_points = oldHitPoints;		// keep the hit points unchanged.
	}
}
//---------- End of function UnitGod::maya_cast_power ----------//


//--------- Begin of function UnitGod::persian_cast_power ---------//
//
void UnitGod::persian_cast_power(Worker* workerPtr, int nationRecno, int divider)
{
	//-- only heal human units belonging to our nation --//

	if( nationRecno == nation_recno && workerPtr->race_id > 0 )
	{
		int changePoints = workerPtr->max_hit_points() / (4+m.random(4));	 // divided by (4 to 7)

		changePoints = MAX( changePoints, 10 );

		workerPtr->change_hit_points( MAX(1, changePoints/divider) );
	}
}
//---------- End of function UnitGod::persian_cast_power ----------//


//--------- Begin of function UnitGod::japanese_cast_power ---------//
//
void UnitGod::japanese_cast_power(Worker* workerPtr, int nationRecno, int divider)
{
	//-- only cast on enemy units -----//

	if( nationRecno != nation_recno && workerPtr->race_id > 0 )
	{
		int changePoints = 7 + m.random(8);		// decrease 7 to 15 loyalty points instantly

		workerPtr->change_loyalty( -MAX(1, changePoints/divider) );
	}
}
//---------- End of function UnitGod::japanese_cast_power ----------//


//--------- Begin of function UnitGod::maya_cast_power ---------//
//
void UnitGod::maya_cast_power(Worker* workerPtr, int nationRecno, int divider)
{
	//-- only cast on mayan units belonging to our nation --//

	if( nationRecno == nation_recno && workerPtr->race_id == RACE_MAYA )
	{
		int changePoints = 15 + m.random(10);		// add 15 to 25 points to its combat level instantly

		int newCombatLevel = workerPtr->combat_level + MAX(1, changePoints/divider);

		if( newCombatLevel > 100 )
			newCombatLevel = 100;

		workerPtr->combat_level = newCombatLevel;
	}
}
//---------- End of function UnitGod::maya_cast_power ----------//


//--------- Begin of function UnitGod::consume_power_pray_points ---------//
//
void UnitGod::consume_power_pray_points()
{
	FirmBase* firmBase = (FirmBase*) firm_array[base_firm_recno];

	err_when( firmBase->firm_id != FIRM_BASE );

	firmBase->pray_points -= god_res[god_id]->power_pray_points;

	if( firmBase->pray_points < 0 )
		firmBase->pray_points = (float) 0;

	hit_points = (short) firmBase->pray_points;
}
//---------- End of function UnitGod::consume_power_pray_points ----------//


#ifdef AMPLUS

//--------- Begin of function UnitGod::egyptian_cast_power ---------//
//
void UnitGod::egyptian_cast_power(int unitRecno, int divider)
{
	// no effect
}
//---------- End of function UnitGod::egyptian_cast_power ----------//

//--------- Begin of function UnitGod::indian_cast_power ---------//
//
void UnitGod::indian_cast_power(int unitRecno, int divider)
{
	Unit* unitPtr = unit_array[unitRecno];

	if( unitPtr->is_visible() && nation_array.should_attack(nation_recno, unitPtr->nation_recno) )
	{
		unitPtr->change_loyalty(-30 + m.random(11));
	}
}
//---------- End of function UnitGod::indian_cast_power ----------//


//--------- Begin of function UnitGod::zulu_cast_power ---------//
//
void UnitGod::zulu_cast_power(int unitRecno, int divider)
{
	// no effect
	Unit* unitPtr = unit_array[unitRecno];

	if( nation_recno == unitPtr->nation_recno && 
		unitPtr->race_id == RACE_ZULU && unitPtr->rank_id != RANK_SOLDIER)
	{
		int changePoints = 30;	// add 15 twice to avoid 130 becomes -126
		if( divider > 2 )
		{
			unitPtr->skill.skill_level += changePoints/divider;
			if( unitPtr->skill.skill_level > 100 )
				unitPtr->skill.skill_level = 100;
		}
		else
		{
			for(int t = 2; t > 0; --t )
			{
				unitPtr->skill.skill_level += changePoints/2/divider;
				if( unitPtr->skill.skill_level > 100 )
					unitPtr->skill.skill_level = 100;
			}
		}
	}
}
//---------- End of function UnitGod::zulu_cast_power ----------//


//--------- Begin of function UnitGod::egyptian_cast_power ---------//
//
void UnitGod::egyptian_cast_power(Worker *workerPtr, int nationRecno, int divider)
{
	// no effect
}
//---------- End of function UnitGod::egyptian_cast_power ----------//


//--------- Begin of function UnitGod::indian_cast_power ---------//
//
void UnitGod::indian_cast_power(Worker *workerPtr, int nationRecno, int divider)
{
	// no effect
}
//---------- End of function UnitGod::indian_cast_power ----------//


//--------- Begin of function UnitGod::zulu_cast_power ---------//
//
void UnitGod::zulu_cast_power(Worker *workerPtr, int nationRecno, int divider)
{
	// no effect
}
//---------- End of function UnitGod::zulu_cast_power ----------//
#endif
