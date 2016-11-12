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

//Filename    : OTOWNRES.CPP
//Description : Town resource object

#include <OSYS.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OIMGRES.h>
#include <ORACERES.h>
#include <OTOWNRES.h>

//---------- define constant ------------//

#define TOWN_LAYOUT_DB 		"TOWNLAY"
#define TOWN_SLOT_DB			"TOWNSLOT"
#define TOWN_BUILD_TYPE_DB "TOWNBTYP"
#define TOWN_BUILD_DB		"TOWNBULD"
#define TOWN_NAME_DB			"TOWNNAME"

//------- Begin of function TownRes::TownRes -----------//

TownRes::TownRes()
{
	init_flag=0;
}
//--------- End of function TownRes::TownRes -----------//


//---------- Begin of function TownRes::init -----------//
//
// This function must be called after a map is generated.
//
void TownRes::init()
{
	deinit();

	//----- open town material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_TOWN.RES";

	res_bitmap.init_imported(str,1);	 // 1-read all into buffer

	//------- load database information --------//

	load_town_slot();			// load_town_slot() must be called first before load_town_layout(), as load_town_layout() accesses town_slot_array
	load_town_layout();
	load_town_build_type();
	load_town_build();
   load_town_name();

	init_flag=1;
}
//---------- End of function TownRes::init -----------//


//---------- Begin of function TownRes::deinit -----------//

void TownRes::deinit()
{
	if( init_flag )
	{
		mem_del(town_layout_array);
		mem_del(town_slot_array);
		mem_del(town_build_array);
		mem_del(town_build_type_array);
		mem_del(town_name_array);
		mem_del(town_name_used_array);

		res_bitmap.deinit();

		init_flag=0;
	}
}
//---------- End of function TownRes::deinit -----------//


//------- Begin of function TownRes::load_town_layout -------//
//
// Read in information from TOWNLAY.DBF.
//
void TownRes::load_town_layout()
{
	TownLayoutRec  *townLayoutRec;
	TownLayout     *townLayout;
	TownSlot			*townSlot;
	int      	  	i, j;
	Database 		*dbTownLayout = game_set.open_db(TOWN_LAYOUT_DB);

	town_layout_count = (short) dbTownLayout->rec_count();
	town_layout_array = (TownLayout*) mem_add( sizeof(TownLayout)*town_layout_count );

	//------ read in town layout info array -------//

	memset( town_layout_array, 0, sizeof(TownLayout) * town_layout_count );

	for( i=0 ; i<town_layout_count ; i++ )
	{
		townLayoutRec = (TownLayoutRec*) dbTownLayout->read(i+1);
		townLayout    = town_layout_array+i;

		townLayout->first_slot_recno = misc.atoi(townLayoutRec->first_slot, TownLayoutRec::FIRST_SLOT_LEN);
		townLayout->slot_count = misc.atoi(townLayoutRec->slot_count, TownLayoutRec::SLOT_COUNT_LEN);

		// ###### begin Gilbert 9/9 ########//
		// townLayout->ground_bitmap_ptr = image_spict.get_ptr( misc.nullify(townLayoutRec->ground_name, TownLayoutRec::GROUND_NAME_LEN) );
		townLayout->ground_bitmap_ptr = image_tpict.get_ptr( misc.nullify(townLayoutRec->ground_name, TownLayoutRec::GROUND_NAME_LEN) );
		// ###### end Gilbert 9/9 ########//

		err_if( townLayout->slot_count > MAX_TOWN_LAYOUT_SLOT )
			err_now( "Error: MAX_TOWN_LAYOUT_SLOT limit exceeded." );

		//----- calculate min_population & max_population -----//

		townSlot = town_slot_array+townLayout->first_slot_recno-1;

		for( j=0 ; j<townLayout->slot_count ; j++, townSlot++ )
		{
			if( townSlot->build_type==TOWN_OBJECT_HOUSE )		// if there is a building in this slot
				townLayout->build_count++;
		}
	}
}
//--------- End of function TownRes::load_town_layout ---------//


//------- Begin of function TownRes::load_town_slot -------//
//
// Read in information from TOWNSLOT.DBF.
//
void TownRes::load_town_slot()
{
	TownSlotRec  	*townSlotRec;
	TownSlot     	*townSlot;
	int      	  	i;
	Database 		*dbTownSlot = game_set.open_db(TOWN_SLOT_DB);

	town_slot_count = (short) dbTownSlot->rec_count();
	town_slot_array = (TownSlot*) mem_add( sizeof(TownSlot)*town_slot_count );

	//------ read in town slot info array -------//

	memset( town_slot_array, 0, sizeof(TownSlot) * town_slot_count );

	for( i=0 ; i<town_slot_count ; i++ )
	{
		townSlotRec = (TownSlotRec*) dbTownSlot->read(i+1);
		townSlot    = town_slot_array+i;

		townSlot->base_x = misc.atoi(townSlotRec->base_x, TownSlotRec::POS_LEN);
		townSlot->base_y = misc.atoi(townSlotRec->base_y, TownSlotRec::POS_LEN);

		townSlot->build_type = misc.atoi(townSlotRec->type_id, TownSlotRec::TYPE_ID_LEN);
		townSlot->build_code = misc.atoi(townSlotRec->build_code, TownSlotRec::BUILD_CODE_LEN);

		err_when( townSlot->build_type == TOWN_OBJECT_FARM &&	
				  (townSlot->build_code < 1 || townSlot->build_code > 9) ); 
	}
}
//--------- End of function TownRes::load_town_slot ---------//


//------- Begin of function TownRes::load_town_build_type -------//
//
// Read in information from TOWNBTYP.DBF.
//
void TownRes::load_town_build_type()
{
	TownBuildTypeRec 	*buildTypeRec;
	TownBuildType    	*buildType;
	int      	  		i;
	Database 			*dbTownBuildType = game_set.open_db(TOWN_BUILD_TYPE_DB);

	town_build_type_count = (short) dbTownBuildType->rec_count();
	town_build_type_array = (TownBuildType*) mem_add( sizeof(TownBuildType)*town_build_type_count );

	//------ read in TownBuildType info array -------//

	memset( town_build_type_array, 0, sizeof(TownBuildType) * town_build_type_count );

	for( i=0 ; i<town_build_type_count ; i++ )
	{
		buildTypeRec = (TownBuildTypeRec*) dbTownBuildType->read(i+1);
		buildType    = town_build_type_array+i;

		buildType->first_build_recno = misc.atoi(buildTypeRec->first_build, TownBuildTypeRec::FIRST_BUILD_LEN);
		buildType->build_count = misc.atoi(buildTypeRec->build_count, TownBuildTypeRec::BUILD_COUNT_LEN);
	}
}
//--------- End of function TownRes::load_town_build_type ---------//


//------- Begin of function TownRes::load_town_build -------//
//
// Read in information from TOWNBULD.DBF.
//
void TownRes::load_town_build()
{
	TownBuildRec  	*townBuildRec;
	TownBuild     	*townBuild;
	int      	  	i;
	uint32_t			bitmapOffset;
	Database 		*dbTownBuild = game_set.open_db(TOWN_BUILD_DB);

	town_build_count = (short) dbTownBuild->rec_count();
	town_build_array = (TownBuild*) mem_add( sizeof(TownBuild)*town_build_count );

	err_when( town_build_count > 255 );			// BYTE is used in TownZone::slot_array[]

	//------ read in town build info array -------//

	memset( town_build_array, 0, sizeof(TownBuild) * town_build_count );

	for( i=0 ; i<town_build_count ; i++ )
	{
		townBuildRec = (TownBuildRec*) dbTownBuild->read(i+1);
		townBuild    = town_build_array+i;

		townBuild->build_type = misc.atoi(townBuildRec->type_id, TownBuildRec::TYPE_ID_LEN);
		townBuild->build_code = misc.atoi(townBuildRec->build_code, TownBuildRec::BUILD_CODE_LEN);
		townBuild->race_id = misc.atoi(townBuildRec->race_id, TownBuildRec::RACE_ID_LEN);

		memcpy( &bitmapOffset, townBuildRec->bitmap_ptr, sizeof(uint32_t) );

		townBuild->bitmap_ptr    = res_bitmap.read_imported(bitmapOffset);
		townBuild->bitmap_width  = *((short*)townBuild->bitmap_ptr);
		townBuild->bitmap_height = *(((short*)townBuild->bitmap_ptr)+1);
	}
}
//--------- End of function TownRes::load_town_build ---------//


//------- Begin of function TownRes::load_town_name -------//
//
// Read in information from TOWNNAME.DBF.
//
// Note: race_res must be initialized before calling this function.
//
void TownRes::load_town_name()
{
	TownNameRec *townNameRec;
	TownName		*townName;
	int      	i;
	Database 	*dbTownName = game_set.open_db(TOWN_NAME_DB);

	town_name_count 		= dbTownName->rec_count();
	town_name_array		= (TownName*) mem_add( sizeof(TownName)*town_name_count );
	town_name_used_array = (unsigned char*) mem_add( sizeof(town_name_used_array[0]) * town_name_count );		// store the used_count separately from town_name_array to faciliate file saving

	memset( town_name_used_array, 0, sizeof(town_name_used_array[0]) * town_name_count );

	//------ read in TownName info array -------//

	int raceId=0;

	for( i=1 ; i<=town_name_count ; i++ )
	{
		townNameRec = (TownNameRec*) dbTownName->read(i);
		townName    = town_name_array+i-1;

		misc.rtrim_fld( townName->name, townNameRec->name, townNameRec->NAME_LEN );

		if( townName->name[0]=='@' )		// next race
		{
			int j;
			for( j=1 ; j<=MAX_RACE ; j++ )
			{
				if( strcmp( race_res[j]->code, townName->name+1 ) == 0 )
					break;
			}

			err_when( j > MAX_RACE );

			if( raceId )
				race_res[raceId]->town_name_count = i-race_res[raceId]->first_town_name_recno;

			raceId = j;
			race_res[raceId]->first_town_name_recno = i+1;
		}
	}

	//-- set the town_name_count of the last  town in TOWNNAME.DBF --//

	race_res[raceId]->town_name_count = i-race_res[raceId]->first_town_name_recno;
}
//--------- End of function TownRes::load_town_name ---------//


//---------- Begin of function TownRes::scan_build -----------//
//
// Set the given slot with a building that fits the given criteria
//
// <int> slotId 	  - the slot id. of the current town section to be set.
// <int> raceId     - one of the building selection criteria
//							 	    (0-any race)
//
// return : <int> townBuildId - the id. of the town building
//
int TownRes::scan_build(int slotId, int raceId)
{
	enum { MAX_SCAN_ID = 100 };

	TownSlot* 		townSlot = town_res.get_slot(slotId);
	TownBuildType* buildType;
	TownBuild*		townBuild;
	int				i, buildRecno, matchCount=0;
	int				scanIdArray[MAX_SCAN_ID];

	//---- get the building type of the slot ------//

	buildType = town_res.get_build_type(townSlot->build_type);

	err_if( buildType->build_count==0 )
		err_here();

	//------ scan_build buildings of the specified type ------//

	buildRecno = buildType->first_build_recno;
	townBuild  = town_res.get_build(buildRecno);	   // the pointer to the first building of the specified type

	for( i=buildType->build_count ; i>0 ; i--, townBuild++, buildRecno++ )
	{
		if( townBuild->build_code == townSlot->build_code )
		{
			if( !raceId || townBuild->race_id == raceId )
			{
				scanIdArray[matchCount] = buildRecno;

				if( ++matchCount >= MAX_SCAN_ID )
					break;
			}
		}
	}

	//--- pick one from those plants that match the criteria ---//

	if( matchCount > 0 )
	{
		int buildId = scanIdArray[misc.random(matchCount)];

		#ifdef DEBUG
			town_res.get_build( buildId );		// get_build() will error if buildId is not valid
		#endif

		return buildId;
	}
	else
		return 0;
}
//---------- End of function TownRes::scan_build -----------//

#ifdef DEBUG

//---------- Begin of function TownRes::get_layout -----------//

TownLayout* TownRes::get_layout(int recNo)
{
	err_if( recNo<1 || recNo>town_layout_count )
		err_now( "TownRes::get_layout()" );

	return town_layout_array+recNo-1;
}
//------------ End of function TownRes::get_layout -----------//


//---------- Begin of function TownRes::get_slot -----------//

TownSlot* TownRes::get_slot(int recNo)
{
	err_if( recNo<1 || recNo>town_slot_count )
		err_now( "TownRes::get_slot()" );

	return town_slot_array+recNo-1;
}
//------------ End of function TownRes::get_slot -----------//


//---------- Begin of function TownRes::get_build_type -----------//

TownBuildType* TownRes::get_build_type(int recNo)
{
	err_if( recNo<1 || recNo>town_build_type_count )
		err_now( "TownRes::get_build_type()" );

	return town_build_type_array+recNo-1;
}
//------------ End of function TownRes::get_build_type -----------//


//---------- Begin of function TownRes::get_build -----------//

TownBuild* TownRes::get_build(int recNo)
{
	err_if( recNo<1 || recNo>town_build_count )
		err_now( "TownRes::get_build()" );

	return town_build_array+recNo-1;
}
//------------ End of function TownRes::get_build -----------//

#endif 

//---------- Begin of function TownRes::get_name -----------//

char* TownRes::get_name(int recNo)
{
	err_if( recNo<1 || recNo>town_name_count )
		err_now( "TownRes::get_name()" );

	return town_name_array[recNo-1].name;
}
//------------ End of function TownRes::get_name -----------//


//--------- Begin of function TownRes::get_new_name_id ----------//
//
int TownRes::get_new_name_id(int raceId)
{
	RaceInfo* raceInfo = race_res[raceId];

	err_when( raceInfo->town_name_used_count > raceInfo->town_name_count );

	int townNameId;

	//----- if all town names have been used already -----//
	//--- scan the town name one by one and pick an unused one ---//

	if( raceInfo->town_name_used_count == raceInfo->town_name_count )
	{
		int nameId = misc.random(raceInfo->town_name_count)+1;		// this is the id. of one race only

		for( int i=raceInfo->town_name_count ; i>0 ; i-- )
		{
			if( ++nameId > raceInfo->town_name_count )
				nameId = 1;

			if( town_name_used_array[raceInfo->first_town_name_recno+nameId-2]==0 )		// -2 is the total of two -1, (one with first_town_name_recno, another with town_name_used_array[]
				break;
		}

		townNameId = raceInfo->first_town_name_recno + nameId - 1;
	}
	else
	{
		raceInfo->town_name_used_count++;

		townNameId = raceInfo->first_town_name_recno + raceInfo->town_name_used_count - 1;
	}

	err_when( townNameId < 1 || townNameId > town_name_count );

	town_name_used_array[townNameId-1]++;

	return townNameId;
}
//--------- End of function TownRes::get_new_name_id ----------//


//--------- Begin of function TownRes::free_name_id ----------//
//
// Free an used name id.
//
void TownRes::free_name_id(int townNameId)
{
	town_name_used_array[townNameId-1]--;
}
//--------- End of function TownRes::free_name_id ----------//
