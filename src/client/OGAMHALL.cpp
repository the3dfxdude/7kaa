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
#include <OGFILE.h>
#include "gettext.h"

//------ Begin of function GameFileArray::disp_hall_of_fame -----//
//
// Display the Hall of Fame
//
void GameFileArray::disp_hall_of_fame()
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
//------- End of function GameFileArray::disp_hall_of_fame -----//


//------ Begin of function HallFame::disp_info -------//
//
// Display a Hall of Fame record
//
// <int> x, y = the location of the information
// <int> pos  = the position of the record.
//
void HallFame::disp_info(int x, int y, int pos)
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
//------- End of function HallFame::disp_info -------//


//------ Begin of function GameFileArray::add_hall_of_fame -----//
//
// Add current game into the hall of hame
//
// <int> totalScore of the player.
//
// return : <int> 1-hall of fame updated
//                0-not updated
//
int GameFileArray::add_hall_of_fame(int totalScore)
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
                     sizeof(HallFame) * (HALL_FAME_NUM-i-1) );
         }

         //-------- record the hall of fame rcord ------//

         hall_fame_array[i].record_data(totalScore);

         //--------- display the hall of fame ----------//

		   write_hall_of_fame();        // must write hall of fame, because it also write the last saved game slot no.

         disp_hall_of_fame();
         return 1;
      }
   }

   return 0;
}
//------- End of function GameFileArray::add_hall_of_fame -----//


//--------- Begin of function HallFame::record_data --------//
//
// Record the hall of fame record_data
//
void HallFame::record_data(int totalScore)
{
	Nation* playerNation = ~nation_array;

	strncpy( player_name, playerNation->king_name(), NationArray::HUMAN_NAME_LEN );
	player_name[NationArray::HUMAN_NAME_LEN] = '\0';

	race_id	  = playerNation->race_id;

	start_year = date.year(info.game_start_date);
	end_year   = info.game_year;

	score  	  = totalScore;
	population = playerNation->all_population();

	difficulty_rating = config.difficulty_rating;
}
//----------- End of function HallFame::record_data ---------//
