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

//Filename    : OMONSRES.CPP
//Description : Monster resource class

#include <OSYS.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OSPRTRES.h>
#include <OUNIT.h>
#include <OUNITRES.h>
#include <OTOWN.h>
#include <OFIRM.h>
#include <OF_MONS.h>
#include <OMONSRES.h>
#include "gettext.h"

//---------- #define constant ------------//

#define MONSTER_DB   "MONSTER"

//------- Begin of function MonsterRes::MonsterRes -----------//

MonsterRes::MonsterRes()
{
   init_flag=0;
}
//--------- End of function MonsterRes::MonsterRes -----------//


//---------- Begin of function MonsterRes::init -----------//
//
// This function must be called after a map is generated.
//
void MonsterRes::init()
{
   deinit();

   //------- load database information --------//

	load_monster_info();

   init_flag=1;
}
//---------- End of function MonsterRes::init -----------//


//---------- Begin of function MonsterRes::deinit -----------//

void MonsterRes::deinit()
{
   if( init_flag )
   {
      mem_del(monster_info_array);
      init_flag=0;
   }
}
//---------- End of function MonsterRes::deinit -----------//


//------- Begin of function MonsterRes::load_monster_info -------//
//
// Read in information of MONSTER.DBF into memory array
//
void MonsterRes::load_monster_info()
{
   MonsterRec  *monsterRec;
   MonsterInfo *monsterInfo;
   int         i;
   Database    *dbMonster = game_set.open_db(MONSTER_DB);

   monster_count      = (short) dbMonster->rec_count();
   monster_info_array = (MonsterInfo*) mem_add( sizeof(MonsterInfo)*monster_count );

   //------ read in monster information array -------//

	memset( monster_info_array, 0, sizeof(MonsterInfo) * monster_count );

   for( i=0 ; i<monster_count ; i++ )
   {
      monsterRec  = (MonsterRec*) dbMonster->read(i+1);
      monsterInfo = monster_info_array+i;

      monsterInfo->monster_id = i+1;

      misc.rtrim_fld( monsterInfo->name, monsterRec->name, monsterRec->NAME_LEN );

		monsterInfo->unit_id      = misc.atoi(monsterRec->unit_id     , monsterRec->UNIT_ID_LEN);
		monsterInfo->level        = monsterRec->level - '0';

		err_when( monsterInfo->level < 1 || monsterInfo->level > MAX_MONSTER_LEVEL );

		misc.rtrim_fld( monsterInfo->firm_build_code, monsterRec->firm_build_code, monsterRec->FIRM_BUILD_CODE_LEN );

		//---- set the monster_id in UnitInfo ----//

		unit_res[monsterInfo->unit_id]->is_monster = 1;
   }
}
//--------- End of function MonsterRes::load_monster_info ---------//


//---------- Begin of function MonsterRes::init_active_monster -----------//

void MonsterRes::init_active_monster()
{
	int monsterId;
	int activeCount=0;
	int loopCount=0;

	//----- reset the active_monster_array ----//

	memset( active_monster_array, 0, sizeof(active_monster_array) );

	//------- in the demo, the active monsters are fixed -----//

	#ifdef DEMO

	active_monster_array[0] = 4;
	active_monster_array[1] = 11;
	active_monster_array[2] = 12;

	#endif

	//-------- set active monsters -------//

	while(1)
	{
		err_when( ++loopCount > 1000 );

		monsterId = misc.random(monster_count)+1;

		//--- check if this monster id. is already in the active monster array ---//

		int i;
		for( i=0 ; i<activeCount ; i++ )
		{
			if( active_monster_array[i] == monsterId )
				break;
		}

		//------- if it's a new one --------//

		if( i==activeCount )
		{
			active_monster_array[activeCount] = monsterId;

			if( ++activeCount == MAX_ACTIVE_MONSTER )
				return;
		}
	}
}
//---------- End of function MonsterRes::init_active_monster -----------//



//---------- Begin of function MonsterRes::operator[] -----------//

MonsterInfo* MonsterRes::operator[](int monsterId)
{
   err_if( monsterId<1 || monsterId>monster_count )
      err_now( "MonsterRes::operator[]" );

   return monster_info_array+monsterId-1;
}
//------------ End of function MonsterRes::operator[] -----------//


//---------- Begin of function MonsterRes::generate -----------//

void MonsterRes::generate(int generateCount)
{
	int monsterId;

	for( int i=0 ; i<generateCount ; i++ )
	{
		monsterId = active_monster_array[misc.random(MAX_ACTIVE_MONSTER)];

		monster_res[monsterId]->create_firm_monster();
	}
}
//---------- End of function MonsterRes::generate -----------//


//------- Begin of function MonsterInfo::create_firm_monster --------//
//
int MonsterInfo::create_firm_monster()
{
   #define MIN_MONSTER_CIVILIAN_DISTANCE  10    // the minimum distance between monster firms and civilian towns & firms

   if( !firm_build_code[0] )     // this monster does not have a home
      return 0;

   //------- locate a home place for the monster group ------//

   FirmInfo* firmInfo = firm_res[FIRM_MONSTER];

   int  xLoc=0, yLoc=0;
   char teraMask = UnitRes::mobile_type_to_mask(UNIT_LAND);

   if( !world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
       MAX_WORLD_Y_LOC-1, firmInfo->loc_width+2, firmInfo->loc_height+2,   // leave at least one location space around the building
       MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, 0, 1, teraMask) )
   {
      return 0;
   }

   //------- don't place it too close to any towns or firms ------//

   for( int townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
   {
      if( town_array.is_deleted(townRecno) )
         continue;

      Town* townPtr = town_array[townRecno];

      if( misc.points_distance(xLoc, yLoc, townPtr->center_x,
          townPtr->center_y) < MIN_MONSTER_CIVILIAN_DISTANCE )
      {
         return 0;
      }
   }

   for( int firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
   {
      if( firm_array.is_deleted(firmRecno) )
         continue;

      Firm* firmPtr = firm_array[firmRecno];

      if( misc.points_distance(xLoc, yLoc, firmPtr->center_x,
          firmPtr->center_y) < MIN_MONSTER_CIVILIAN_DISTANCE )
      {
         return 0;
      }
   }

   return build_firm_monster(xLoc+1, yLoc+1);
}
//---------- End of function MonsterInfo::create_firm_monster -----------//


//------- Begin of function MonsterInfo::build_firm_monster --------//
// [int] fullHitPoints - build with full hit points (default = 1)
//
int MonsterInfo::build_firm_monster(int xLoc, int yLoc, int fullHitPoints)
{
   //----- if this monster has a home building, create it first -----//

   FirmMonster* firmPtr=NULL;

   int firmRecno = firm_array.build_firm(xLoc, yLoc, 0, FIRM_MONSTER, firm_build_code);

   if( !firmRecno )
      return 0;

   firmPtr = (FirmMonster*) firm_array[firmRecno];

	if(fullHitPoints)
		firmPtr->complete_construction();
	else
	{
		firmPtr->hit_points = (float) 0.1;
		firmPtr->under_construction = 1;
	}

   firmPtr->set_king(monster_id, 100);

   //-------- create monster generals ---------//

	int generalCount = misc.random(2)+1;      // 1 to 3 generals in a monster firm

	if( misc.random(5)==0 )		// 20% chance of having 3 generals.
		generalCount=3;

   for( int i=0 ; i<generalCount ; i++ )
      firmPtr->recruit_general();

   firmPtr->monster_id = monster_id;

   return firmRecno;
}
//---------- End of function MonsterInfo::build_firm_monster -----------//


//--------- Begin of function MonsterRes::stop_attack_nation -------//

void MonsterRes::stop_attack_nation(short nationRecno)
{
   FirmMonster* firmMonster;

   for(int i=firm_array.size(); i>0; i--)
   {
      if( firm_array.is_deleted(i) || firm_array[i]->firm_id != FIRM_MONSTER )
         continue;

      firmMonster = (FirmMonster*) firm_array[i];

      firmMonster->reset_hostile_nation(nationRecno);
   }
}
//----------- End of function MonsterRes::stop_attack_nation ---------//


//---------- Begin of function MonsterRes::get_monster_by_unit_id -----------//

MonsterInfo* MonsterRes::get_monster_by_unit_id(int unitId)
{
	int i;
	for( i=0 ; i<monster_count ; i++ )
	{
		MonsterInfo* monsterInfo = monster_info_array+i;
		if( monsterInfo->unit_id == unitId )
			return monsterInfo;
	}
	return NULL;
}
//------------ End of function MonsterRes::get_monster_by_unit_id -----------//
