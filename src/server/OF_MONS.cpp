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

//Filename    : OF_MONS.CPP
//Description : Firm Airport

#include <OINFO.h>
#include <OVGA.h>
#include <ODATE.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OSITE.h>
#include <OFONT.h>
#include <OBUTTON.h>
#include <OPOWER.h>
#include <OTOWN.h>
#include <OU_MONS.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OMONSRES.h>
#include <OF_MONS.h>

//----------- Define constant ------------//

#define MONSTER_SOLDIER_COMBAT_LEVEL_DIVIDER		2
static char current_monster_action_mode;

//--------- Begin of function FirmMonster::init_derived ---------//
//
void FirmMonster::init_derived()
{
	monster_general_count = 0;

	// ##### patch begin Gilbert 21/1 #######//
	// monster_king.monster_id = 0;
	memset( &monster_king, 0, sizeof(monster_king) );
	// ##### patch end Gilbert 21/1 #######//
	memset( monster_general_array, 0, sizeof(monster_general_array) );
	waiting_soldier_count = 0;

	defending_king_count = 0;
	defending_general_count = 0;
	defending_soldier_count = 0;

	monster_nation_relation = 0;

	//----------------------------------------//
	// Set monster agressiveness. It affects:
	//
	// -the number of defenders will be called out at one time.
	//----------------------------------------//

	defend_target_recno = 0;

	patrol_unit_count   = 0;

	//-- these vars must be initialized here instead of in FirmMonster::FirmMonster() for random seed sync during load game --//

	monster_aggressiveness = 20 + m.random(50);		// 20 to 70
}
//----------- End of function FirmMonster::init_derived -----------//


//--------- Begin of function FirmMonster::~FirmMonster ---------//
//
FirmMonster::~FirmMonster()
{
	if( sys.signal_exit_flag )
		return;

	int goldAmount = 800 * (monster_res[monster_id]->level*30 + m.random(50)) / 100;

	site_array.add_site( center_x, center_y, SITE_GOLD_COIN, goldAmount );
	site_array.ai_get_site_object();		// ask AI units to get the gold coins
}
//----------- End of function FirmMonster::~FirmMonster -----------//


//--------- Begin of function FirmMonster::deinit_derived ---------//
//
void FirmMonster::deinit_derived()
{
	if( sys.signal_exit_flag )
		return;

	//-------- mobilize all monsters in the firm --------//

	int loopCount=0;

	if( monster_king.monster_id )
		mobilize_king();

	while( monster_general_count > 0 )
	{
		if(!mobilize_general(1))
			break;

		err_when( loopCount++ > 100 );
	}

	clear_defense_mode();
}
//----------- End of function FirmMonster::deinit_derived -----------//


//------- Begin of function FirmMonster::firm_name -----------//
//
char* FirmMonster::firm_name()
{
	static String str;

#if(defined(SPANISH))
	str  = "Guarida ";
	str += monster_res[monster_id]->name;
#elif(defined(FRENCH))
	str  = "Antre des ";
	str += monster_res[monster_id]->name;
#else
	// GERMAN, US
	str  = monster_res[monster_id]->name;
	str += translate.process(" Lair");
#endif

	return str;
}
//--------- End of function FirmMonster::firm_name -----------//


//--------- Begin of function FirmMonster::put_info ---------//
//
void FirmMonster::put_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !config.show_ai_info && nation_recno!=nation_array.player_recno )
		return;

	disp_monster_info(INFO_Y1+54, refreshFlag);
}
//----------- End of function FirmMonster::put_info -----------//


//--------- Begin of function FirmMonster::detect_info ---------//
//
void FirmMonster::detect_info()
{
	if( detect_basic_info() )
		return;

	if( !config.show_ai_info && nation_recno!=nation_array.player_recno )
		return;
}
//----------- End of function FirmMonster::detect_info -----------//


//--------- Begin of function FirmMonster::disp_monster_info ---------//
//
void FirmMonster::disp_monster_info(int dispY1, int refreshFlag)
{
	return;

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+22 );

	int x=INFO_X1+4, y=dispY1+3;
}
//----------- End of function FirmMonster::disp_monster_info -----------//


//--------- Begin of function FirmMonster::next_day ---------//
//
void FirmMonster::next_day()
{
	//-------- validate patrol unit --------//

	validate_patrol_unit();

	//---- the monster boss recruit new monsters ----//

	if( info.game_date%10 == firm_recno%10 )
		recruit_soldier();

	//----- monsters recover hit points -------//

	if( info.game_date%15 == firm_recno%15 )			// once a week
		recover_hit_points();

	//------ monster thinks about expansion -------//

	if( config.monster_type == OPTION_MONSTER_OFFENSIVE )
	{
		if( info.game_date%30 == firm_recno%30 && m.random(3)==0 )
			recruit_general();
/*
		if( info.game_date%90 == firm_recno%90 )
			think_attack_neighbor();
*/
		//------ attack human towns and firms randomly -----//

		if( info.game_date > info.game_start_date + 1000 &&	// only start attacking 3 years after the game starts so the human can build up things
			 info.game_date%30 == firm_recno%30 &&
			 m.random( firm_res[FIRM_MONSTER]->total_firm_count*6 )==0 )		// it will expand slower when there are already a lot of the monster structures on the map
		{
			think_attack_human();
		}

		//--------- think expansion ---------//

		if( info.game_date%180 == firm_recno%180 &&
			 m.random( firm_res[FIRM_MONSTER]->total_firm_count*10 )==0 )		// it will expand slower when there are already a lot of the monster structures on the map
		{
			think_expansion();
		}
	}
}
//----------- End of function FirmMonster::next_day -----------//


//------- Begin of function FirmMonster::recover_hit_points -------//
//
void FirmMonster::recover_hit_points()
{
   //------ recover the king's hit points -------//

	if( monster_king.hit_points < monster_king.max_hit_points )
		monster_king.hit_points++;

	//------ recover the generals' hit points -------//

	for( int i=0 ; i<monster_general_count ; i++ )
	{
		if( monster_general_array[i].hit_points < monster_general_array[i].max_hit_points )
			monster_general_array[i].hit_points++;
	}
}
//--------- End of function FirmMonster::recover_hit_points -------//


//--------- Begin of function FirmMonster::assign_unit ---------//
//
// Mobilized defender units are assigned back to the firm.
//
void FirmMonster::assign_unit(int unitRecno)
{
	Unit*		 unitPtr  = unit_array[unitRecno];
	UnitInfo* unitInfo = unit_res[unitPtr->unit_id];

	err_when( !unitInfo->is_monster );

	switch( unitPtr->rank_id )
	{
		case RANK_KING:
			set_king(unitPtr->get_monster_id(), unitPtr->skill.combat_level);
			break;

		case RANK_GENERAL:
			add_general(unitRecno);
			break;

		case RANK_SOLDIER:
			add_soldier(unitPtr->leader_unit_recno);
			break;
	}

	//--------- the unit disappear in firm -----//

	unit_array.disappear_in_firm(unitRecno);
}
//----------- End of function FirmMonster::assign_unit -----------//


//--------- Begin of function FirmMonster::set_king ---------//
//
// Set the monster king of this firm.
//
void FirmMonster::set_king(int monsterId, int combatLevel)
{
	monster_king.monster_id 	= monsterId;
	monster_king.set_combat_level(combatLevel);
	monster_king.hit_points 	= monster_king.max_hit_points;
}
//----------- End of function FirmMonster::set_king -----------//


//--------- Begin of function FirmMonster::add_general ---------//
//
// Add a general to the firm, this function is called when a
// mobilized monster general is assigned back to the firm.
//
void FirmMonster::add_general(int generalUnitRecno)
{
	if(monster_general_count>=MAX_MONSTER_GENERAL_IN_FIRM)
		return;

	Unit*		 unitPtr  = unit_array[generalUnitRecno];
	UnitInfo* unitInfo = unit_res[unitPtr->unit_id];

	MonsterInFirm* monsterInFirm = monster_general_array+monster_general_count;

	monsterInFirm->monster_id 	  = unitPtr->get_monster_id(); 	// contribution is used for storing the monster id. temporary
	monsterInFirm->set_combat_level(unitPtr->skill.combat_level);
	monsterInFirm->hit_points 	  = (int) unitPtr->hit_points;

	if( monsterInFirm->hit_points == 0 )		// 0.? will become 0 in (float) to (int) conversion
		monsterInFirm->hit_points = 1;

	monsterInFirm->soldier_monster_id = unitPtr->get_monster_soldier_id();		// skill id is used for storing the soldier monster id temporarily
	monsterInFirm->soldier_count   	 = 0;

	monsterInFirm->mobile_unit_recno = generalUnitRecno; 	// unit recno of this monster when it is a mobile unit
																			// this is only used as a reference for soldiers to find their leaders
	monster_general_count++;

	//----- check if there are any soldiers waiting for this general ----//
	//
	// These are soldiers who follow the general to go out to defend
	// against the attack but then went back to the firm sooner
	// than the general does.
	//
	//-------------------------------------------------------------------//

	for( int i=0 ; i<waiting_soldier_count ; i++ )
	{
		//--- if this waiting soldier was led by this general ---//

		if( waiting_soldier_array[i] == generalUnitRecno )
		{
			monsterInFirm->soldier_count++;

			err_when( waiting_soldier_count > MAX_WAITING_SOLDIER );

			m.del_array_rec(waiting_soldier_array, waiting_soldier_count, sizeof(waiting_soldier_array[0]), i+1);
			waiting_soldier_count--;
		}
	}
}
//----------- End of function FirmMonster::add_general -----------//


//--------- Begin of function FirmMonster::add_soldier ---------//
//
// <int> generalUnitRecno - the unit recno of the general leading
//									 this soldier.
//
void FirmMonster::add_soldier(int generalUnitRecno)
{
	//----- check if the soldier's leading general is here ----//

	for( int i=0 ; i<monster_general_count ; i++ )
	{
		if( monster_general_array[i].mobile_unit_recno == generalUnitRecno )
		{
			if(monster_general_array[i].soldier_count>=MAX_SOLDIER_PER_GENERAL)
				return;

			monster_general_array[i].soldier_count++;
			return;
		}
	}

	//---- if not, put the soldier into the waiting list ----//

	if( waiting_soldier_count < MAX_WAITING_SOLDIER )	// if the list is full, the unit disappear in air
	{
		waiting_soldier_array[waiting_soldier_count++] = generalUnitRecno;	// the soldier is waiting for this general.
	}
}
//----------- End of function FirmMonster::add_soldier -----------//


//--------- Begin of function FirmMonster::recruit_general ---------//
//
// Recruit general monsters.
//
// [int] soldierCount - the no. of soldiers to be created with this general.
//								(default: randomly from 1 to MAX_MONSTER_PER_GENERAL)
//
int FirmMonster::recruit_general(int soldierCount)
{
	err_when( monster_general_count < 0 || monster_general_count > MAX_MONSTER_GENERAL_IN_FIRM );

	if( monster_general_count >= MAX_MONSTER_GENERAL_IN_FIRM * monster_aggressiveness / 100 )
		return 0;

	if( !monster_king.monster_id )
		return 0;

	//---------- recruit the general now ----------//

	MonsterInFirm* monsterInFirm = monster_general_array+monster_general_count;

	int combatLevel = 40 + m.random(30);		// 40 to 70

	monsterInFirm->monster_id 	  = monster_king.monster_id;
	monsterInFirm->set_combat_level(combatLevel);
	monsterInFirm->hit_points 	  = monsterInFirm->max_hit_points;

	monsterInFirm->soldier_monster_id = monster_king.monster_id;

	if( soldierCount >= 0 )
		monsterInFirm->soldier_count = soldierCount;
	else
		monsterInFirm->soldier_count = m.random(MAX_SOLDIER_PER_GENERAL/2)+1;

	monster_general_count++;

	return 1;
}
//----------- End of function FirmMonster::recruit_general -----------//


//--------- Begin of function FirmMonster::recruit_soldier ---------//
//
// Recruit soldier monsters.
//
void FirmMonster::recruit_soldier()
{
	MonsterInFirm* monsterInFirm = monster_general_array;

	for( int i=0 ; i<monster_general_count ; i++, monsterInFirm++ )
	{
		if( monsterInFirm->soldier_count < MAX_SOLDIER_PER_GENERAL &&
			 m.random(3) > 0 )		// 2/3 chance of recruiting a soldier
		{
			monsterInFirm->soldier_count++;
		}
	}
}
//----------- End of function FirmMonster::recruit_soldier -----------//


//--------- Begin of function FirmMonster::mobilize_king ---------//
//
// The king himself does not lead any soldiers.
//
int FirmMonster::mobilize_king()
{
	if( !mobilize_monster( monster_king.monster_id, RANK_KING, monster_king.combat_level, monster_king.hit_points ) )
		return 0;

	monster_king.monster_id = 0;

	return 1;
}
//----------- End of function FirmMonster::mobilize_king ---------//


//--------- Begin of function FirmMonster::mobilize_general ---------//
//
// Mobilize monster generals. Soldiers need by the general is also mobilized.
//
// <int> generalId 		 - id. of the general.
// [int] mobilizeSoldier - whether also mobilize soldiers this general
//									commands. (default: 1)
//
// Return: <int> the no. of monsters have been mobilized.
//
int FirmMonster::mobilize_general(int generalId, int mobilizeSoldier)
{
	err_when( generalId < 1 || generalId > monster_general_count );

	MonsterInFirm* monsterInFirm = monster_general_array + generalId - 1;

	//------ mobilize the monster general ------//

	int generalUnitRecno = mobilize_monster( monsterInFirm->monster_id, RANK_GENERAL, monsterInFirm->combat_level, monsterInFirm->hit_points );

	if( !generalUnitRecno )
		return 0;

	unit_array[generalUnitRecno]->set_monster_soldier_id(monsterInFirm->soldier_monster_id);

	int mobilizedCount = 1;

	patrol_unit_array[0] = generalUnitRecno;
	patrol_unit_count = 1;

	//------ mobilize soldiers commanded by the monster general ------//

	if( mobilizeSoldier )
	{
		for( int i=0 ; i<monsterInFirm->soldier_count ; i++ )
		{
			//--- the combat level of its soldiers ranges from 25% to 50% of the combat level of the general ---//

			int soldierCombatLevel = monsterInFirm->combat_level/MONSTER_SOLDIER_COMBAT_LEVEL_DIVIDER + m.random(monsterInFirm->combat_level/MONSTER_SOLDIER_COMBAT_LEVEL_DIVIDER);

			int unitRecno = mobilize_monster( monsterInFirm->soldier_monster_id, RANK_SOLDIER, soldierCombatLevel );

			if( unitRecno )
			{
				unit_array[unitRecno]->leader_unit_recno = generalUnitRecno;
				mobilizedCount++;

				patrol_unit_array[patrol_unit_count++] = unitRecno;

				if(patrol_unit_count==MAX_SOLDIER_PER_GENERAL+1)
					break;
			}
			else
				break; // no space for init_sprite
		}
   }

	//---- delete the monster general record from the array ----//

	err_when( monster_general_count > MAX_MONSTER_GENERAL_IN_FIRM );

	m.del_array_rec(monster_general_array, monster_general_count, sizeof(MonsterInFirm), generalId);

	monster_general_count--;

	return mobilizedCount;
}
//----------- End of function FirmMonster::mobilize_general ---------//


//--------- Begin of function FirmMonster::mobilize_monster ---------//
//
// <int> monsterId   = id. of the monster to be mobilized
// <int> rankId		= rank id. of the monster. 
// <int> combatLevel = the combat level of the monster
// [int] hitPoints	= set the hit points of the monster to this
//							  (default: hit points of the monster is set to the MAX hit points when it is first created.)
//
// return: <int> the unit recno of the mobile unit created.
//
int FirmMonster::mobilize_monster(int monsterId, int rankId, int combatLevel, int hitPoints)
{
	MonsterInfo* monsterInfo = monster_res[monsterId];
	UnitInfo*    unitInfo    = unit_res[monsterInfo->unit_id];

	//------- locate a space first --------//

	int			xLoc=center_x, yLoc=center_y;
	SpriteInfo* spriteInfo = sprite_res[unitInfo->sprite_id];

	if( !world.locate_space( xLoc, yLoc, xLoc, yLoc, spriteInfo->loc_width, spriteInfo->loc_height, unitInfo->mobile_type ) )
		return 0;

	//---------- add the unit now -----------//

	int unitRecno = unit_array.add_unit( unitInfo->unit_id, 0,
						 rankId, 0, xLoc, yLoc );

	UnitMonster* monsterPtr = (UnitMonster*) unit_array[unitRecno];

	monsterPtr->set_mode(UNIT_MODE_MONSTER, firm_recno);
	monsterPtr->set_combat_level(combatLevel);
	monsterPtr->set_monster_id(monsterId);
	monsterPtr->set_monster_action_mode(current_monster_action_mode);

	if( hitPoints )
	{
		monsterPtr->hit_points = (float) hitPoints;
		err_when( hitPoints > monsterPtr->max_hit_points );
	}
	else
		monsterPtr->hit_points = monsterPtr->max_hit_points;

	//-----------------------------------------------------//
	// enable unit defend mode
	//-----------------------------------------------------//
	if(firm_recno) // 0 when firm is ready to be deleted
	{
		monsterPtr->stop2();
		monsterPtr->action_mode2 = ACTION_MONSTER_DEFEND_DETECT_TARGET;
		monsterPtr->action_para2 = MONSTER_DEFEND_DETECT_COUNT;

		monsterPtr->action_misc = ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO;
		monsterPtr->action_misc_para = firm_recno;
	}

	return unitRecno;
}
//----------- End of function FirmMonster::mobilize_monster -----------//


//--------- Begin of function FirmMonster::being_attacked ---------//
//
// This function is called by Unit::hit_firm()
//
// <int> attackerUnitRecno - recno of the unit attacking this firm.
//
void FirmMonster::being_attacked(int attackerUnitRecno)
{
	int attackerNationRecno = unit_array[attackerUnitRecno]->nation_recno;

	//--- increase reputation of the nation that attacks monsters ---//

	if( attackerNationRecno )
	{
		nation_array[attackerNationRecno]->change_reputation(REPUTATION_INCREASE_PER_ATTACK_MONSTER);
		set_hostile_nation(attackerNationRecno);		// also set hostile with the nation
	}

	//------ MAX no. of defender it should call out -----//

	int maxDefender = MAX_MONSTER_IN_FIRM * monster_aggressiveness / 200;

	if( total_defender() >= maxDefender )	// only mobilize new ones when the MAX defender no. has been reached yet
		return;

	current_monster_action_mode = MONSTER_ACTION_DEFENSE;

	//---- mobilize monster general to defend against the attack ----//

	if( monster_general_count > 0 )
	{
		int mobilizedCount = mobilize_general( m.random(monster_general_count)+1 );

		if(mobilizedCount)
		{
			defending_general_count++;
			defending_soldier_count += mobilizedCount-1;
		}
	}

	else if( monster_king.monster_id )
	{
		if( mobilize_king() )
			defending_king_count++;
	}

	defend_target_recno = attackerUnitRecno;
}
//----------- End of function FirmMonster::being_attacked -----------//


//-------- Begin of function FirmMonster::clear_defense_mode -------//
//
// This function is called when the firm monster is destroyed.
//
void FirmMonster::clear_defense_mode()
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

		//------ reset the monster's defense mode -----//

		if(unitPtr->in_monster_defend_mode() && unitPtr->action_misc==ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO &&
			unitPtr->action_misc_para==firm_recno)
		{
			unitPtr->clear_monster_defend_mode();
			((UnitMonster*)unitPtr)->set_monster_action_mode(MONSTER_ACTION_STOP);
		}

		//--- if this unit belongs to this firm, reset its association with this firm ---//

		if( unitPtr->unit_mode == UNIT_MODE_MONSTER &&
			 unitPtr->unit_mode_para == firm_recno )
		{
			unitPtr->unit_mode_para = 0;
		}
	}
}
//----------- End of function FirmMonster::clear_defense_mode -----------//


//-------- Begin of function FirmMonster::reduce_defender_count -------//
//
// A defender unit moves back into the firm, reducing the defender count.
//
// <int> rankId - rank id. of the defender unit
//
void FirmMonster::reduce_defender_count(int rankId)
{
	switch(rankId)
	{
		case RANK_KING:
			defending_king_count--;
			if( defending_king_count < 0 )			//**BUGHERE
				defending_king_count = 0;
			err_when( defending_king_count < 0 );
			break;

		case RANK_GENERAL:
			defending_general_count--;
			if( defending_general_count < 0 )			//**BUGHERE
				defending_general_count = 0;
			err_when( defending_general_count < 0 );
			break;

		case RANK_SOLDIER:
			defending_soldier_count--;
			if( defending_soldier_count < 0 )			//**BUGHERE
				defending_soldier_count = 0;
			err_when( defending_soldier_count < 0 );
			break;
	}

	if( total_defender()==0 )
		monster_nation_relation = 0;
}
//------ End of function FirmMonster::reduce_defender_count ---------//


//-------- Begin of function MonsterInFirm::set_combat_level -------//

void MonsterInFirm::set_combat_level(int combatLevel)
{
	UnitInfo* unitInfo = unit_res[monster_res[monster_id]->unit_id];

	combat_level = combatLevel;

	max_hit_points = (int) unitInfo->hit_points * combatLevel / 100;
}
//--------- End of function MonsterInFirm::set_combat_level --------//


//-------- Begin of function FirmMonster::total_combat_level -------//
//
int FirmMonster::total_combat_level()
{
	MonsterInFirm* monsterInFirm = monster_general_array;
	int				totalCombatLevel=50;		// for the structure 

	for( int i=0 ; i<monster_general_count ; i++, monsterInFirm++ )
	{
		totalCombatLevel += monsterInFirm->hit_points +
			 (monsterInFirm->combat_level * 2 / MONSTER_SOLDIER_COMBAT_LEVEL_DIVIDER)			// *2 because total_combat_level() actually takes hit_points instead of combat_level
			 * 3 / 2 * monsterInFirm->soldier_count;		// *3/2 because the function use 100% + random(100%) for the combat level, so we take 150% as the average
	}

	return totalCombatLevel;
}
//------ End of function FirmMonster::total_combat_level ---------//


//-------- Begin of function FirmMonster::can_assign_monster -------//
//
// Return whether the given monster can be assigned to this firm.
//
// It checks if the type of the monster and the type of the monster
// structure are compatible.
//
int FirmMonster::can_assign_monster(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];
	int   monsterId = unitPtr->get_monster_id();

	return strcmp( firm_res.get_build(firm_build_id)->build_code,	// can assign if the build code are the same
			 monster_res[monsterId]->firm_build_code ) == 0;
}
//------ End of function FirmMonster::can_assign_monster ---------//


//-------- Begin of function FirmMonster::set_hostile_nation -------//
void FirmMonster::set_hostile_nation(int nationRecno)
{
	if(nationRecno==0)
		return;

	err_when(nationRecno>7); // only 8 bits
	monster_nation_relation |= (0x1 << nationRecno);
}
//------ End of function FirmMonster::set_hostile_nation ---------//


//-------- Begin of function FirmMonster::reset_hostile_nation -------//
void FirmMonster::reset_hostile_nation(int nationRecno)
{
	if(nationRecno==0)
		return;

	err_when(nationRecno>7); // only 8 bits
	monster_nation_relation &= ~(0x1 << nationRecno);
}
//------ End of function FirmMonster::reset_hostile_nation ---------//


//-------- Begin of function FirmMonster::is_hostile_nation -------//
// return 1 for hostile nation
// return 0 otherwise
//
int FirmMonster::is_hostile_nation(int nationRecno)
{
	if(nationRecno==0)
		return 0;

	err_when(nationRecno>7); // only 8 bits
	return (monster_nation_relation & (0x1 << nationRecno));
}
//------ End of function FirmMonster::is_hostile_nation ---------//


//------- Begin of function FirmMonster::validate_patrol_unit ---------//
//
void FirmMonster::validate_patrol_unit()
{
	if(patrol_unit_count<=0)
		return;

	int 	unitRecno;
	Unit* unitPtr;

	for( int i=patrol_unit_count ; i>0 ; i-- )
	{
		unitRecno = patrol_unit_array[i-1];

		if( unit_array.is_deleted(unitRecno) ||
			 (unitPtr=unit_array[unitRecno])->is_visible()==0 )
		{
			err_when( patrol_unit_count > MAX_SOLDIER_PER_GENERAL+1 );

			m.del_array_rec( patrol_unit_array, patrol_unit_count, sizeof(patrol_unit_array[0]), i );

			err_when( patrol_unit_count==0 );		// it's already 0

			patrol_unit_count--;
		}
	}

	err_when(patrol_unit_count<0);
	if(patrol_unit_count==0 && total_defender()==0)
		monster_nation_relation = 0;
	else if(patrol_unit_count<0)
		patrol_unit_count = 0;
}
//-------- End of function FirmMonster::validate_patrol_unit ---------//


//------- Begin of function FirmMonster::think_attack_neighbor -------//
//
int FirmMonster::think_attack_neighbor()
{
	//-- don't attack new target if some mobile monsters are already attacking somebody --//

	if( patrol_unit_count > 0 )
		return 0;

	//-------- only attack if we have enough generals ---------//

	int generalCount=0;

	MonsterInFirm* monsterInFirm = monster_general_array;
	int				totalCombatLevel=0;

	//--- count the number of generals commanding at least 5 soldiers ---//

	int i;
	for( i=0 ; i<monster_general_count ; i++, monsterInFirm++ )
	{
		if( monsterInFirm->soldier_count >= 5 )
			generalCount++;
	}

	if( generalCount<=1 )	// don't attack if there is only one general in the firm
		return 0;

	//------ look for neighbors to attack ------//

	int		 xOffset, yOffset;
	int		 xLoc, yLoc;
	int		 attackFlag=0;
	FirmInfo* firmInfo = firm_res[firm_id];
	Location* locPtr;

	int scanLocWidth  = MONSTER_ATTACK_NEIGHBOR_RANGE*2;
	int scanLocHeight = MONSTER_ATTACK_NEIGHBOR_RANGE*2;
	int scanLimit = scanLocWidth * scanLocHeight;
	short targetNation;

	for(i=firmInfo->loc_width*firmInfo->loc_height+1; i<=scanLimit; i++)
	{
		m.cal_move_around_a_point(i, scanLocWidth, scanLocHeight, xOffset, yOffset);

		xLoc = center_x + xOffset;
		yLoc = center_y + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if(locPtr->is_firm())
		{
			Firm *firmPtr = firm_array[locPtr->firm_recno()];
			if(firmPtr->nation_recno)
			{
				targetNation = firmPtr->nation_recno;
				attackFlag = 1;
				xLoc = firmPtr->loc_x1;
				yLoc = firmPtr->loc_y1;
				break;
			}
		}
		else if(locPtr->is_town())
		{
			Town *townPtr = town_array[locPtr->town_recno()];
			if(townPtr->nation_recno)
			{
				targetNation = townPtr->nation_recno;
				attackFlag = 1;
				xLoc = townPtr->loc_x1;
				yLoc = townPtr->loc_y1;
				break;
			}
		}
	}

	if( !attackFlag )
		return 0;

	//------- attack the civilian now --------//
	current_monster_action_mode = MONSTER_ACTION_ATTACK;

	mobilize_general( m.random(monster_general_count)+1 );

	if( patrol_unit_count > 0 )
	{
		//### begin alex 16/9 ###//
		set_hostile_nation(targetNation);
		//#### end alex 16/9 ####//
		// ##### patch begin Gilbert 5/8 #######//
		unit_array.attack(xLoc, yLoc, 0, patrol_unit_array, patrol_unit_count, COMMAND_AI, 0);
		// ##### patch end Gilbert 5/8 #######//
		return 1;
	}
	else
		return 0;
}
//-------- End of function FirmMonster::think_attack_neighbor -------//


//-------- Begin of function FirmMonster::think_expansion -------//
//
int FirmMonster::think_expansion()
{
	#define MIN_GENERAL_EXPAND_NUM 3

	if(patrol_unit_count>0)
		return 0;

	if(!monster_king.monster_id || monster_general_count<MIN_GENERAL_EXPAND_NUM)
		return 0;

	//--- count the number of generals commanding 8 soldiers ----//

	int generalCount=0;

	MonsterInFirm* monsterInFirm = monster_general_array;

	for( int i=0 ; i<monster_general_count ; i++, monsterInFirm++ )
	{
		if( monsterInFirm->soldier_count == MAX_SOLDIER_PER_GENERAL )
			generalCount++;
	}

	if( generalCount < MIN_GENERAL_EXPAND_NUM )	// don't expand if the no. of generals is less than 3
		return 0;

	//------------- locate space to build monster firm randomly -------------//
	#define EXPAND_FIRM_DISTANCE	30
	#define FREE_SPACE_DISTANCE	3

	MonsterInfo* monsterInfo = monster_res[monster_king.monster_id];
   FirmInfo* firmInfo = firm_res[FIRM_MONSTER];
   char	teraMask = UnitRes::mobile_type_to_mask(UNIT_LAND);
   int	xLoc1 = MAX(0, loc_x1-EXPAND_FIRM_DISTANCE);
	int	yLoc1 = MAX(0, loc_y1-EXPAND_FIRM_DISTANCE);
	int	xLoc2 = MIN(MAX_WORLD_X_LOC-1, loc_x2+EXPAND_FIRM_DISTANCE);
	int	yLoc2 = MIN(MAX_WORLD_Y_LOC-1, loc_y2+EXPAND_FIRM_DISTANCE);

   if( !world.locate_space_random(xLoc1, yLoc1, xLoc2, yLoc2,
		 firmInfo->loc_width+FREE_SPACE_DISTANCE*2,
		 firmInfo->loc_height+FREE_SPACE_DISTANCE*2,   // leave at least 3 location space around the building
		 (xLoc2-xLoc1+1)*(yLoc2-yLoc1+1), 0, 1, teraMask) )
	{
		return 0;
	}

	monster_general_count--;
	//monsterInFirm = monster_general_array + monster_general_count;
	//unit_array.disappear_in_firm(monsterInFirm->mobile_unit_recno);

	return monsterInfo->build_firm_monster(xLoc1+FREE_SPACE_DISTANCE, yLoc1+FREE_SPACE_DISTANCE, 1);		//1-full hit points
}
//------ End of function FirmMonster::think_expansion ---------//


//------- Begin of function FirmMonster::think_attack_human -------//
//
int FirmMonster::think_attack_human()
{
	//-- don't attack new target if some mobile monsters are already attacking somebody --//

	if( patrol_unit_count > 0 )
		return 0;

	//-------- only attack if we have enough generals ---------//

	int generalCount=0;

	MonsterInFirm* monsterInFirm = monster_general_array;
	int				totalCombatLevel=0;

	//--- count the number of generals commanding at least 5 soldiers ---//

	int i;
	for( i=0 ; i<monster_general_count ; i++, monsterInFirm++ )
	{
		if( monsterInFirm->soldier_count >= 5 )
			generalCount++;
	}

	if( generalCount<=1 )	// don't attack if there is only one general in the firm
		return 0;

	//------ look for neighbors to attack ------//

	int   firmRecno, townRecno;
	int   targetXLoc= -1, targetYLoc, targetNationRecno=0;
	Firm* firmPtr;
	Town* townPtr;

	for(i=1 ; i<100 ; i++ )
	{
		//----- randomly pick a firm ------//

		firmRecno = m.random(firm_array.size()) + 1;

		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->region_id == region_id )
		{
			targetXLoc = firmPtr->loc_x1;
			targetYLoc = firmPtr->loc_y1;
			targetNationRecno = firmPtr->nation_recno;
			break;
		}

		//----- randomly pick a town ------//

		townRecno = m.random(town_array.size()) + 1;

		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( townPtr->nation_recno && townPtr->region_id == region_id )
		{
			targetXLoc = townPtr->loc_x1;
			targetYLoc = townPtr->loc_y1;
			targetNationRecno = townPtr->nation_recno;
			break;
		}
	}

	if( targetXLoc == -1 )		// no target selected
		return 0;

	//------- attack the civilian now --------//

	current_monster_action_mode = MONSTER_ACTION_ATTACK;

	mobilize_general( m.random(monster_general_count)+1 );

	if( patrol_unit_count > 0 )
	{
		set_hostile_nation(targetNationRecno);
		// ##### patch begin Gilbert 5/8 #######//
		unit_array.attack(targetXLoc, targetYLoc, 0, patrol_unit_array, patrol_unit_count, COMMAND_AI, 0);
		// ##### patch end Gilbert 5/8 #######//
		return 1;
	}
	else
		return 0;
}
//-------- End of function FirmMonster::think_attack_human -------//

