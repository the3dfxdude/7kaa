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

//Filename    : OGAMHALL.CPP
//Description : Hall of Fame

#include <OGAMHALL.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OVGALOCK.h>
#include <ODATE.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <ORACERES.h>
#include <OGAME.h>
#include <ONATION.h>
#include <dbglog.h>
#include "gettext.h"

#include <string.h> // for strncpy

#define HALL_OF_FAME_FILE_NAME  "HALLFAME.DAT"

DBGLOG_DEFAULT_CHANNEL(HallOfFame);


HallOfFame::HallOfFame()
	: hall_fame_array{},
	  last_savegame_file_name{}
{
}


void HallOfFame::init()
{
	read_hall_of_fame();
}


void HallOfFame::deinit()
{
	write_hall_of_fame();
}


void HallOfFame::set_last_savegame_file_name(const char* fileName) {
	strncpy(last_savegame_file_name, fileName, MAX_PATH);
	last_savegame_file_name[MAX_PATH] = '\0';
}


//------- Begin of function HallOfFame::read_hall_of_fame ------//
//
int HallOfFame::read_hall_of_fame()
{
	char full_path[MAX_PATH+1];
	int  rc;
	File file;

	if (!misc.path_cat(full_path, sys.dir_config, HALL_OF_FAME_FILE_NAME, MAX_PATH))
	{
		ERR("Path to the hall of fame too long.\n");
		return 0;
	}

	if( !misc.is_file_exist(full_path) )
		return 0;

	rc = file.file_open(full_path, 0, 1);   // 0=don't handle error itself
											// 1=allow the writing size and the read size to be different
	if( !rc )
		return 0;
	// 1=allow the writing size and the read size to be different
	//--------- Read Hall of Fame ----------//

	if( rc )
		rc = file.file_read( hall_fame_array, sizeof(HallFameEntry) * HALL_FAME_NUM );

	//------ read last saved game file name ------//

	if( rc )
		rc = file.file_read( last_savegame_file_name, MAX_PATH+1 );

	file.file_close();

	return rc;
}
//--------- End of function HallOfFame::read_hall_of_fame ------//


//------- Begin of function HallOfFame::write_hall_of_fame ------//
//
int HallOfFame::write_hall_of_fame()
{
	char full_path[MAX_PATH+1];
	int  rc;
	File file;

	if (!misc.path_cat(full_path, sys.dir_config, HALL_OF_FAME_FILE_NAME, MAX_PATH))
	{
		ERR("Path to the hall of fame too long.\n");
		return 0;
	}

	rc = file.file_create( full_path, 0, 1 );  // 0=don't handle error itself

	if( !rc )
		return 0;
	// 1=allow the writing size and the read size to be different
	//--------- Write Hall of Fame ----------//

	if( rc )
		rc = file.file_write( hall_fame_array, sizeof(HallFameEntry) * HALL_FAME_NUM );

	//------ write last saved game file name ------//

	if( rc )
		rc = file.file_write( last_savegame_file_name, MAX_PATH+1 );

	file.file_close();

	return rc;
}
//--------- End of function HallOfFame::write_hall_of_fame ------//


//------ Begin of function HallOfFame::disp_hall_of_fame -----//
//
// Display the Hall of Fame
//
void HallOfFame::disp_hall_of_fame()
{
	vga_util.disp_image_file("HALLFAME");

	//---------- display hall of fame records ------------//

	int i;
	int x=120, y=116;

	for( i=0 ; i<HALL_FAME_NUM ; i++, y+=76 )
	{
		hall_fame_array[i].disp_info( x, y, i+1 );
	}

	mouse.wait_press(60);		// 60 seconds to time out

	vga_util.finish_disp_image_file();
}
//------- End of function HallOfFame::disp_hall_of_fame -----//


//------ Begin of function HallOfFame::add_hall_of_fame -----//
//
// Add current game into the hall of hame
//
// <int> totalScore of the player.
//
// return : <int> 1-hall of fame updated
//                0-not updated
//
int HallOfFame::add_hall_of_fame(int totalScore)
{
	//-------- insert the record -----------//

	int i;

	for( i=0 ; i<HALL_FAME_NUM ; i++ )
	{
		if( totalScore > hall_fame_array[i].score )
		{
			//---------- move and insert the data --------//

			if( i < HALL_FAME_NUM-1 )      // it is not the last record
			{
				memmove( hall_fame_array+i+1, hall_fame_array+i,
					sizeof(HallFameEntry) * (HALL_FAME_NUM-i-1) );
			}

			//-------- record the hall of fame rcord ------//

			hall_fame_array[i].record_data(totalScore);

			//--------- display the hall of fame ----------//

			write_hall_of_fame();

			disp_hall_of_fame();
			return 1;
		}
	}

	return 0;
}
//------- End of function HallOfFame::add_hall_of_fame -----//


//------ Begin of function HallFameEntry::disp_info -------//
//
// Display a Hall of Fame record
//
// <int> x, y = the location of the information
// <int> pos  = the position of the record.
//
void HallFameEntry::disp_info(int x, int y, int pos)
{
	if( !start_year )    // no information
		return;

	//------------------------------------------------------//
	//
	// e.g. 1. [Image] King Trevor Chan
	//    	  [     ] Score : 150    Population : 1000    Period : 1001-1030
	//
	//------------------------------------------------------//

	Font* fontPtr;

	#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
		fontPtr = &font_hall;
	#else
		fontPtr = &font_std;
	#endif

	String str;
	int    y2 = y+17;

	//----------------------------------------//

	str  = pos;
	str += ".";

	fontPtr->put( x, y, str );

	x += 16;

	//----------------------------------------//

	str  = _("King");
	str += " ";
	str += player_name;

	fontPtr->put( x, y, str );

	//----------------------------------------//

	str  = _("Score : ");
	str += score;

	fontPtr->put( x, y2, str );

	//----------------------------------------//

	str  = _("Population");
	str += " : ";
	str += population;

	fontPtr->put( x+110, y2, str );

	//----------------------------------------//

	#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
		x-=10;			// position adjustment for the German version
	#endif

	str  = _("Period : ");
	str += misc.num_to_str(start_year);     // without adding comma separators
	str += "-";
	str += misc.num_to_str(end_year);

	fontPtr->put( x+260, y2, str );

	//----------------------------------------//

	str  = _("Difficulty : ");
	str += difficulty_rating;

	fontPtr->put( x+420, y2, str );
}
//------- End of function HallFameEntry::disp_info -------//


//--------- Begin of function HallFameEntry::record_data --------//
//
// Record the hall of fame record_data
//
void HallFameEntry::record_data(int totalScore)
{
	Nation* playerNation = ~nation_array;

	strncpy( player_name, playerNation->king_name(), HUMAN_NAME_LEN );
	player_name[HUMAN_NAME_LEN] = '\0';

	race_id	  = playerNation->race_id;

	start_year = date.year(info.game_start_date);
	end_year   = info.game_year;

	score  	  = totalScore;
	population = playerNation->all_population();

	difficulty_rating = config.difficulty_rating;
}
//----------- End of function HallFameEntry::record_data ---------//
