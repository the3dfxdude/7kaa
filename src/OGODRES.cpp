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

//Filename    : OGODRES.CPP
//Description : God resource class

#include <OSYS.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OSPRTRES.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OUNITRES.h>
#include <OU_GOD.h>
#include <OF_BASE.h>
#include <OGODRES.h>

//---------- #define constant ------------//

#define GOD_DB   "GOD"

//------- Begin of function GodRes::GodRes -----------//

GodRes::GodRes()
{
	init_flag=0;
}
//--------- End of function GodRes::GodRes -----------//


//---------- Begin of function GodRes::init -----------//
//
// This function must be called after a map is generated.
//
void GodRes::init()
{
	deinit();

	//------- load database information --------//

	load_god_info();

	init_flag=1;
}
//---------- End of function GodRes::init -----------//


//---------- Begin of function GodRes::deinit -----------//

void GodRes::deinit()
{
	if( init_flag )
	{
		mem_del(god_info_array);
		init_flag=0;
	}
}
//---------- End of function GodRes::deinit -----------//


//------- Begin of function GodRes::load_god_info -------//
//
// Read in information of GOD.DBF into memory array
//
void GodRes::load_god_info()
{
	GodRec  *godRec;
	GodInfo *godInfo;
	int      i;
	Database *dbGod = game_set.open_db(GOD_DB);

	god_count      = (short) dbGod->rec_count();
	god_info_array = (GodInfo*) mem_add( sizeof(GodInfo)*god_count );

	//------ read in god information array -------//

	memset( god_info_array, 0, sizeof(GodInfo) * god_count );

	for( i=0 ; i<god_count ; i++ )
	{
		godRec  = (GodRec*) dbGod->read(i+1);
		godInfo = god_info_array+i;

		godInfo->god_id = i+1;

		godInfo->race_id = misc.atoi(godRec->race_id, godRec->RACE_ID_LEN);
		godInfo->unit_id = misc.atoi(godRec->unit_id, godRec->UNIT_ID_LEN);

		godInfo->exist_pray_points = misc.atoi(godRec->exist_pray_points, godRec->PRAY_POINTS_LEN);
		godInfo->power_pray_points = misc.atoi(godRec->power_pray_points, godRec->PRAY_POINTS_LEN);

		godInfo->can_cast_power   = godRec->can_cast_power == '1';
		godInfo->cast_power_range = misc.atoi(godRec->cast_power_range, godRec->CAST_POWER_RANGE_LEN);
	}
}
//--------- End of function GodRes::load_god_info ---------//


//------- Begin of function GodRes::enable_know_all -------//
//
// Make all god creatures known to this nation. (A cheating function)
//
void GodRes::enable_know_all(int nationRecno)
{
	for( int i=1 ; i<=god_res.god_count ; i++ )
	{
		god_res[i]->enable_know(nationRecno);
	}
}
//--------- End of function GodRes::enable_know_all ---------//


//------- Begin of function GodRes::init_nation_know -------//
//
// Initialize the god know status of a new nation.
//
void GodRes::init_nation_know(int nationRecno)
{
	for( int i=1 ; i<=god_res.god_count ; i++ )
	{
		god_res[i]->disable_know(nationRecno);
	}
}
//--------- End of function GodRes::init_nation_know ---------//


//------- Begin of function GodInfo::enable_know -------//
//
// Make this god known to this nation.
//
void GodInfo::enable_know(int nationRecno)
{
	nation_know_array[nationRecno-1] = 1;

	nation_array[nationRecno]->know_base_array[race_id-1] = 1;		// enable the nation to build the fortress of power
}
//--------- End of function GodInfo::enable_know ---------//


//------- Begin of function GodInfo::disable_know -------//
//
// Make this god known to this nation.
//
void GodInfo::disable_know(int nationRecno)
{
	nation_know_array[nationRecno-1] = 0;

	nation_array[nationRecno]->know_base_array[race_id-1] = 0;		// enable the nation to build the fortress of power
}
//--------- End of function GodInfo::disable_know ---------//


//------- Begin of function GodInfo::invoke -------//
//
// <int> firmRecno  - the firm recno of the seat of power which invokes this god.
// <int> xLoc, yLoc - the x and y location of the god
//
// return: <int> the recno of the God unit created.
//
short GodInfo::invoke(int firmRecno, int xLoc, int yLoc)
{
	FirmBase* firmBase = (FirmBase*) firm_array[firmRecno];

	err_when( firmBase->firm_id != FIRM_BASE );

	//------- create the god unit --------//

	SpriteInfo* spriteInfo = sprite_res[unit_res[unit_id]->sprite_id];

	if( !world.locate_space( &xLoc, &yLoc, xLoc, yLoc, spriteInfo->loc_width, spriteInfo->loc_height, UNIT_AIR ) )
		return 0;

	//---------- add the god unit now -----------//

	short unitRecno = unit_array.add_unit( unit_id, firmBase->nation_recno,
							RANK_SOLDIER, 0, xLoc, yLoc );

	//------- set vars of the God unit ----------//

	UnitGod* unitGod = (UnitGod*) unit_array[unitRecno];

	unitGod->god_id			 = god_id;
	unitGod->base_firm_recno = firmRecno;
	unitGod->hit_points 		 = (short) firmBase->pray_points;

	return unitRecno;
}
//--------- End of function GodInfo::invoke ---------//


//------- Begin of function GodRes::is_god_unit -------//
//
// Whether the given unit is a god unit or not.
//
int GodRes::is_god_unit(int unitId)
{
	for( int i=1 ; i<=god_res.god_count ; i++ )
	{
		if( god_res[i]->unit_id == unitId )
			return 1;
	}

	return 0;
}
//--------- End of function GodRes::is_god_unit ---------//


//---------- Begin of function GodRes::operator[] -----------//

GodInfo* GodRes::operator[](int godId)
{
	err_if( godId<1 || godId>god_count )
		err_now( "GodRes::operator[]" );

	return god_info_array+godId-1;
}

//------------ End of function GodRes::operator[] -----------//


