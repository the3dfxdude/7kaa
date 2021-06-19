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

//Filename    : ORACERES.CPP
//Description : Race resource object

#include <OSYS.h>
#include <OGAMESET.h>
#include <OSKILL.h>
#include <OUNITRES.h>
#include <ORACERES.h>
#include <locale.h>
#include <gettext.h>

//---------- #define constant ------------//

#define RACE_DB   		"RACE"
#define RACE_NAME_DB		"RACENAME"

//------- Begin of function RaceRes::RaceRes -----------//

RaceRes::RaceRes()
{
	init_flag=0;
}
//--------- End of function RaceRes::RaceRes -----------//


//---------- Begin of function RaceRes::init -----------//
//
// This function must be called after a map is generated.
//
void RaceRes::init()
{
	deinit();

	//----- open unit bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_RACE.RES";

	res_bitmap.init_imported(str, 1);  // 1-don't read all into buffer

	//------- load database information --------//

	load_race_info();
	load_name();

	init_flag=1;
}
//---------- End of function RaceRes::init -----------//


//---------- Begin of function RaceRes::deinit -----------//

void RaceRes::deinit()
{
	if( init_flag )
	{
		mem_del(race_info_array);
		mem_del(name_array);
		mem_del(name_used_array);

		init_flag=0;
	}
}
//---------- End of function RaceRes::deinit -----------//


//------- Begin of function RaceRes::load_race_info -------//
//
// Read in information of RACE.DBF into memory array
//
void RaceRes::load_race_info()
{
	RaceRec  *raceRec;
	RaceInfo *raceInfo;
	int      i, unitId;
	uint32_t	bitmapOffset;
	Database *dbRace = game_set.open_db(RACE_DB);

	race_count      = (short) dbRace->rec_count();
	race_info_array = (RaceInfo*) mem_add( sizeof(RaceInfo)*race_count );

	//------ read in race information array -------//

	memset( race_info_array, 0, sizeof(RaceInfo) * race_count );

	for( i=0 ; i<race_count ; i++ )
	{
		raceRec  = (RaceRec*) dbRace->read(i+1);
		raceInfo = race_info_array+i;

		raceInfo->race_id = i+1;

		misc.rtrim_fld( raceInfo->code, raceRec->code, raceRec->CODE_LEN );
		misc.rtrim_fld( raceInfo->name, raceRec->name, raceRec->NAME_LEN );
		misc.rtrim_fld( raceInfo->adjective, raceRec->adjective, raceRec->ADJECTIVE_LEN );

		memcpy( &bitmapOffset, raceRec->icon_bitmap_ptr, sizeof(uint32_t) );

		raceInfo->icon_bitmap_ptr = res_bitmap.read_imported(bitmapOffset);

		err_when( !raceInfo->icon_bitmap_ptr );

		for( unitId=1 ; unitId<=MAX_UNIT_TYPE ; unitId++ )
		{
			if( unit_res[unitId]->race_id == i+1 )
			{
				raceInfo->basic_unit_id = unitId;
				break;
			}
		}
	}
}
//--------- End of function RaceRes::load_race_info ---------//


//------- Begin of function RaceRes::load_name -------//
//
// Read in information from RACENAME.DBF.
//
// Note: race_res must be initialized before calling this function.
//
void RaceRes::load_name()
{
	#define MAX_SINGLE_RACE_NAME	255		// cannot be more than 255 in each name group, as Unit::name_id is an <int> and half of it is for the first name and another half of it is for the last name

	RaceNameRec *raceNameRec;
	RaceName		*raceName;
	int      	i, j;
	Database 	*dbRaceName = game_set.open_db(RACE_NAME_DB);

	name_count = (short) dbRaceName->rec_count();
	name_array = (RaceName*) mem_add( sizeof(RaceName)*name_count );
	name_used_array = (unsigned char*) mem_add( sizeof(name_used_array[0])*name_count );

	memset( name_used_array, 0, sizeof(name_used_array[0])*name_count );

	//------ read in RaceName info array -------//

	int raceId=0, isFirstName;

	for( i=1 ; i<=name_count ; i++ )
	{
		raceNameRec = (RaceNameRec*) dbRaceName->read(i);
		raceName    = name_array+i-1;

		misc.rtrim_fld( raceName->name, raceNameRec->name, raceNameRec->NAME_LEN );
		// The default STD.SET uses a different code page than the used fonts.
		misc.dos_encoding_to_win(raceName->name, raceName->NAME_LEN);

		if( raceName->name[0]=='@' )
		{
			if( raceId )
			{
				if( isFirstName )
					race_res[raceId]->first_name_count = i-race_res[raceId]->first_first_name_id;
				else
					race_res[raceId]->last_name_count = i-race_res[raceId]->first_last_name_id;

				err_when( race_res[raceId]->first_name_count > MAX_SINGLE_RACE_NAME );
				err_when( race_res[raceId]->last_name_count  > MAX_SINGLE_RACE_NAME );
			}

			//----- get the race id. of the following names -----//

			for( j=1 ; j<=MAX_RACE ; j++ )
			{
				if( strcmp( race_res[j]->code, raceName->name+2 )==0 )
				{
					raceId = j;
					break;
				}
			}

			err_when( j>MAX_RACE );

			//----------------------------------------------//

			isFirstName = raceName->name[1]=='1';		// whether the following names are first names

			if( isFirstName )
				race_res[raceId]->first_first_name_id = i+1;		// next record following the "@RACECODE" is the first name record
			else
				race_res[raceId]->first_last_name_id = i+1;		// next record following the "@RACECODE" is the first name record
		}
	}

	//------- finish up the last race in the list ------//

	if( raceId )
	{
		if( isFirstName )
			race_res[raceId]->first_name_count = i-race_res[raceId]->first_first_name_id;
		else
			race_res[raceId]->last_name_count = i-race_res[raceId]->first_last_name_id;

		err_when( race_res[raceId]->first_name_count > MAX_SINGLE_RACE_NAME );
		err_when( race_res[raceId]->last_name_count  > MAX_SINGLE_RACE_NAME );
	}
}
//--------- End of function RaceRes::load_name ---------//


//---------- Begin of function RaceRes::is_same_race -----------//
//
// Return whether the two given race ids are the same.
//
// This function is called instead of direct in-line comparsion
// because it is easier to research all lines that compare races
// by keyword searching is_same_race().
//
int RaceRes::is_same_race(int raceId1, int raceId2)
{
	return raceId1 == raceId2;
}
//---------- End of function RaceRes::is_same_race -----------//


//----- Start of function RaceInfo::get_new_name_id ------//
//
// Return an unused name id. and set the used_count of the
// first and first name to 1.
//
uint16_t RaceInfo::get_new_name_id()
{
	//---------- get the first name ----------//

	int i;
	int firstNameId = misc.random(first_name_count)+1;

	for( i=1 ; i<=first_name_count ; i++ )
	{
		if( ++firstNameId > first_name_count )
			firstNameId = 1;

		//--- try to get an unused first name -----//
		//--- if all names have been used (when i>first_name_count), use the first selected random name id. --//

		if( !race_res.name_used_array[first_first_name_id+firstNameId-2] )
			break;
	}

	int nameRecno = first_first_name_id+firstNameId-1;

	err_when( nameRecno < 1 || nameRecno > race_res.name_count );

	race_res.name_used_array[nameRecno-1]++;

	//---------- get the last name ----------//

	int lastNameId;

	if( last_name_count==0 )  // if there is no last name for this race, add Roman letter as the last name
	{
		lastNameId = race_res.name_used_array[first_first_name_id+firstNameId-2];
	}
	else		// this race has last names
	{
		lastNameId = misc.random(last_name_count)+1;

		for( i=1 ; i<=last_name_count ; i++ )
		{
			if( ++lastNameId > last_name_count )
				lastNameId = 1;

			//--- try to get an unused last name -----//
			//--- if all names have been used, use the first selected random name id. --//

			if( !race_res.name_used_array[first_last_name_id+lastNameId-2] )
				break;
		}

		nameRecno = first_last_name_id+lastNameId-1;

		err_when( nameRecno < 1 || nameRecno > race_res.name_count );

		race_res.name_used_array[nameRecno-1]++;
	}

	//--- nameId is a combination of first & last name id. ----//

	err_when( firstNameId < 1 || firstNameId > first_name_count );
	err_when( last_name_count>0 && (lastNameId  < 1 || lastNameId  > last_name_count) );

	return (firstNameId<<8) + lastNameId;
}
//------ End of function RaceInfo::get_new_name_id -------//


//----- Start of function RaceInfo::free_name_id ------//
//
// Free an used name id.
//
// Unit names are freed when they settle into a town.
// But unit names are not freed when they are killed.
//
void RaceInfo::free_name_id(uint16_t nameId)
{
	err_when( !nameId );

	int firstNameId = (nameId>>8);
	int nameRecno = first_first_name_id+firstNameId-1;

	err_when( firstNameId < 1 || firstNameId > first_name_count );
	err_when( nameRecno   < 1 || nameRecno   > race_res.name_count );

	race_res.name_used_array[nameRecno-1]--;

	if( last_name_count > 0 )				// some races do not have last names
	{
		int lastNameId  = (nameId&0xFF);
		int nameRecno = first_last_name_id+lastNameId-1;

		err_when( lastNameId < 1 || lastNameId > last_name_count );
		err_when( nameRecno  < 1 || nameRecno  > race_res.name_count );

		race_res.name_used_array[nameRecno-1]--;
	}
}
//------ End of function RaceInfo::free_name_id -------//


//----- Start of function RaceInfo::use_name_id ------//
//
// Claim the use a specific name id.
//
void RaceInfo::use_name_id(uint16_t nameId)
{
	int firstNameId = (nameId>>8);
	int nameRecno = first_first_name_id+firstNameId-1;

	err_when( firstNameId < 1 || firstNameId > first_name_count );
	err_when( nameRecno   < 1 || nameRecno   > race_res.name_count );

	race_res.name_used_array[nameRecno-1]++;

	if( last_name_count > 0 )				// some races do not have last names
	{
		int lastNameId  = (nameId&0xFF);
		int nameRecno = first_last_name_id+lastNameId-1;

		err_when( lastNameId < 1 || lastNameId > last_name_count );
		err_when( nameRecno  < 1 || nameRecno  > race_res.name_count );

		race_res.name_used_array[nameRecno-1]++;
	}
}
//------ End of function RaceInfo::use_name_id -------//


//-------- Start of function RaceInfo::get_name -------------//
//
// <uint16_t> nameId - higher byte - first name id, lower byte - last name id.
// [int]  nameType - 0-full name, 1-first name only, 2-last name only
//
const char* RaceInfo::get_name(uint16_t nameId, int nameType)
{
	static String str;

	if( nameId==0 )
	{
		return "";
	}
	else
	{
		if( nameType != 2 )		// 2-is for last name only
		{
			int firstNameId = (nameId>>8);

			err_when( firstNameId > first_name_count );

			int nameRecno = first_first_name_id+firstNameId-1;

			err_when( nameRecno < 1 || nameRecno > race_res.name_count );

			str = race_res.name_array[nameRecno-1].name;

			if( nameType==1 )		// first name only
				return str;
		}
		else
		{
			if( last_name_count==0 )	// if this race does not have last name
				return "";

			str = "";
		}

		//--------- last name ----------//

		int lastNameId = (nameId&0xFF);

		if( last_name_count==0 )  // if there is no last name for this race
		{
			if( lastNameId > 1 )	  // no need to display roman letter "I" for the first one
			{
				if( str.len() > 0 )
					str += " ";

				str += misc.roman_number(lastNameId);
			}
		}
		else
		{
			err_when( lastNameId > last_name_count );

			int nameRecno = first_last_name_id+lastNameId-1;

			err_when( nameRecno < 1 || nameRecno > race_res.name_count );

			if( str.len() > 0 )
				str += " ";

			str += race_res.name_array[nameRecno-1].name;
		}

		return str;
	}
}
//--------- End of function RaceInfo::get_name ---------------//


//-------- Start of function RaceInfo::get_single_name -------------//
//
// Return a single word name. If there are last names for this race,
// return the last name, otherwise, return the first name.
//
// <uint16_t> nameId - higher byte - first name id, lower byte - last name id.
//
const char* RaceInfo::get_single_name(uint16_t nameId)
{
	switch( race_id )
	{
   	case RACE_NORMAN:
		case RACE_VIKING:
#if (MAX_RACE >= 10)
		case RACE_INDIAN:
		case RACE_ZULU:
#endif
			return get_name(nameId, 1);		// 1-first name only

		case RACE_CHINESE:
		case RACE_JAPANESE:
			return get_name(nameId, 2);		// 2-last name only

		default:
			return get_name(nameId);			// the whole name
	}
}
//--------- End of function RaceInfo::get_single_name ---------------//


//---------- Begin of function RaceRes::operator[] -----------//

RaceInfo* RaceRes::operator[](int raceId)
{
	err_if( raceId<1 || raceId>race_count )
		err_now( "RaceRes::operator[]" );

	return race_info_array+raceId-1;
}

//------------ End of function RaceRes::operator[] -----------//
