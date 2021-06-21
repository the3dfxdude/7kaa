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

//Filename    : OGAME.CPP
//Description : Main Game Object

#include <ALL.h>
#include <COLCODE.h>
#include <OSYS.h>
#include <OVGA.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OBUTTON.h>
#include <OUNITALL.h>
#include <OBULLET.h>
#include <OTERRAIN.h>
#include <OPLANT.h>
#include <OWORLD.h>
#include <OPOWER.h>
#include <OSITE.h>
#include <ORACERES.h>
#include <OWALLRES.h>
#include <OTECHRES.h>
#include <OGODRES.h>
#include <OMONSRES.h>
#include <OTOWN.h>
#include <OTownNetwork.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <OIMGRES.h>
#include <OINFO.h>
#include <OSPRITE.h>
#include <OGAMESET.h>
#include <OGAME.h>
#include <OREBEL.h>
#include <OSPY.h>
#include <OBATTLE.h>
#include <ONEWS.h>
#include <OWEATHER.h>
#include <OHILLRES.h>
#include <OTALKRES.h>
#include <OSNOWRES.h>
#include <OSNOWG.h>
#include <OEXPMASK.h>
#include <OSE.h>
#include <OSERES.h>
#include <OROCKRES.h>
#include <OROCK.h>
#include <OEFFECT.h>
#include <OAUDIO.h>
#include <OMUSIC.h>
#include <OTORNADO.h>
#include <OWARPT.h>
// ##### begin Gilbert 2/10 #######//
#include <OFIRMDIE.h>
// ##### end Gilbert 2/10 #######//

//---------------- DETECT_SPREAD ----------------//
//
// Spread out 2 tiles on all direction when detecting sprite.
// A bigger value is required for bigger sprite.
// this should be a number big enough to cover the biggest
// sprite in the game.
//
// It would be equal to the size of the biggest
// sprite in the game + 1 (+1 for the difference
// between (next_x & cur_x).
//
//-----------------------------------------------//

//-------- Begin of function Game::Game --------//
//
Game::Game()
{
   init_flag = 0;
   game_mode = GAME_PREGAME;

	init_remap_table();        // initialize color remap table
}
//--------- End of function Game::Game ---------//


//-------- Begin of function Game::init --------//
//
// Note: all functions called in this function cannot call
//			misc.random(). Otherwise, it will cause random seed
//		   sync error.
//
// [int] loadGameCall - weather this function is called
//                      when a game is being loaded
//                      (default: 0)
//
int Game::init(int loadGameCall)
{
   if( init_flag )
		deinit();

	int originalRandomSeed = misc.get_random_seed();

	music.stop();

	// ----- set waiting cursor -------- //
	int oldCursor = mouse_cursor.get_icon();
	mouse_cursor.set_icon(CURSOR_WAITING);

	//------- init game data class ---------//

	game_set.open_set(1);             // open the default game set

	char tpictFile[] = DIR_RES"I_TPICT?.RES";
	*(strstr( tpictFile, "?")) = '0' + config.terrain_set;
	image_tpict.init(tpictFile,1,0);		// config.terrain_set dependent, must load before town_res.init and terrain_res.init
	terrain_res.init();
	plant_res.init();
	tech_res.init();
	god_res.init();

	sprite_res.init();                              // sprite resource object must been initialized after game_set as it relies on game_set for info.
	sprite_frame_res.init();
	unit_res.init();
   monster_res.init();

	raw_res.init();
	race_res.init();
	firm_res.init();
	// ##### begin Gilbert 2/10 #######//
	firm_die_res.init();
	// ##### end Gilbert 2/10 #######//
	town_res.init();
	hill_res.init();
	snow_res.init();
	rock_res.init();
	explored_mask.init(vga.vga_color_table);
   se_res.init1();
	se_res.init2(&se_ctrl);
	talk_res.init();

	//------- init game data class ---------//

	nation_array.init();
	firm_array.init();
	// ##### begin Gilbert 2/10 #######//
	firm_die_array.init();
	// ##### end Gilbert 2/10 #######//
	town_array.init();
	town_network_array.init();
	unit_array.init();
	bullet_array.init();
	rebel_array.init();
	spy_array.init();
	site_array.init();
	rock_array.init();
	dirt_array.init();
	effect_array.init();
	tornado_array.init();
	war_point_array.init();

   //------ init game surface class ----------//

	power.init();
   world.init();
   battle.init();
   news_array.init();

	if( !loadGameCall )
		info.init();   // it reads in the panel texture and copy it to vga_back's buffer

   //---------------------------------------------//

	int quakeFreq;

	if( config.random_event_frequency )
	{
		quakeFreq = 2000 - config.random_event_frequency * 400
						+ info.random_seed%300;
	}
	else
	{
		quakeFreq = 0x0fffffff;
	}

	weather.init_date(info.game_year, info.game_month, info.game_day, config.latitude, quakeFreq);

	//---------------------------------------------//

	weather_forecast[0] = weather;
   weather_forecast[0].next_day();

   for(int foreDay=1; foreDay < MAX_WEATHER_FORECAST; ++foreDay)
   {
      weather_forecast[foreDay] = weather_forecast[foreDay-1];
      weather_forecast[foreDay].next_day();
	}

	snow_ground_array.init(weather.snow_scale(), info.game_year);

	//------------ run demo ------------//

	err_when( originalRandomSeed != misc.get_random_seed() );

	// ----- restore from waiting cursor -------- //

	mouse_cursor.restore_icon(oldCursor);

	game_has_ended = 0;

	if( !loadGameCall )
		memset(scenario_file_name, 0, FilePath::MAX_FILE_PATH+1);

	init_flag=1;

	return 1;
}
//--------- End of function Game::init ---------//


//-------- Begin of function Game::deinit --------//
//
// [int] loadGameCall - weather this function is called
//                      when a game is being loaded
//                      (default: 0)
//
void Game::deinit(int loadGameCall)
{
	if( !init_flag )
		return;

	power.disable();     // disable power, which handle mouse inputs

	music.stop();
	audio.stop_wav();

	//----- set waiting cursor -------- //

	int oldCursor = mouse_cursor.get_icon();
	mouse_cursor.set_icon(CURSOR_WAITING);

	//------- deinit game data class ---------//

	nation_array.deinit();
	firm_array.deinit();
	// ##### begin Gilbert 2/10 #######//
	firm_die_array.deinit();
	// ##### end Gilbert 2/10 #######//
	town_network_array.deinit();
	town_array.deinit();
	unit_array.deinit();
	bullet_array.deinit();
	rebel_array.deinit();
	spy_array.deinit();
	region_array.deinit();
	site_array.deinit();
	rock_array.deinit();
	dirt_array.deinit();
	effect_array.deinit();
	tornado_array.deinit();
	war_point_array.deinit();

	//------ deinit game surface class -------//

	world.deinit();
	battle.deinit();
	news_array.deinit();

	if( !loadGameCall )
		info.deinit();

	//------- deinit game data class ---------//

	image_tpict.deinit();
	terrain_res.deinit();
	plant_res.deinit();
	tech_res.deinit();
	god_res.deinit();
	monster_res.deinit();

	sprite_res.deinit();
	sprite_frame_res.deinit();
	unit_res.deinit();

	raw_res.deinit();
	race_res.deinit();
	firm_res.deinit();
	// ##### begin Gilbert 2/10 #######//
	firm_die_res.deinit();
	// ##### end Gilbert 2/10 #######//
	town_res.deinit();
	hill_res.deinit();
	snow_res.deinit();
	rock_res.deinit();
	explored_mask.deinit();
	se_res.deinit();
	talk_res.deinit();

	game_set.close_set();           // close the game set

	//----- restore from waiting cursor -------- //

	mouse_cursor.restore_icon(oldCursor);

	init_flag=0;
}
//--------- End of function Game::deinit ---------//


//--------- Begin of function Game::init_remap_table --------//

void Game::init_remap_table()
{
	//------------- Define constant ------------//

	#define FIRST_REMAP_KEY       0xE0        // the source color code of the colors to be remapped
	#define REMAP_KEY_COUNT       8

	//-------- define color remap scheme -------//

	static ColorRemapMethod remap_method_array[] =
	{
      { 0xBC, 0xDC },   // the first remap table is for independent units
      { 0xA0, 0xC0 },   // following are eight remap table for each color code
      { 0xA4, 0xC4 },
      { 0xA8, 0xC8 },
      { 0xAC, 0xCC },
      { 0xB0, 0xD0 },
		{ 0xB4, 0xD4 },
      { 0xB8, 0xD8 },
      { 0xBC, 0xDC },
   };

   //---- define the main color code for each color scheme ----//

   static int main_color_array[] =
   {
      0xDC,
      0xC0,
      0xC4,
      0xC8,
      0xCC,
      0xD0,
      0xD4,
      0xD8,
	};

	//-------- initialize color remap table -------//

	int         i, j;
	ColorRemap *colorRemap = color_remap_array;

	for( i=0 ; i<MAX_COLOR_SCHEME+1 ; i++, colorRemap++ )    // MAX_COLOR_SCHEME+1, +1 for independent units
	{
      colorRemap->main_color = main_color_array[i];

      for( j=0 ; j<256 ; j++ )
         colorRemap->color_table[j] = j;

      for( j=0 ; j<4 ; j++ )
         colorRemap->color_table[FIRST_REMAP_KEY+j] = (char) (remap_method_array[i].primary_color+j);

      for( j=0 ; j<4 ; j++ )
         colorRemap->color_table[FIRST_REMAP_KEY+4+j] = (char) (remap_method_array[i].secondary_color+j);

//    for( j=0 ; j<4 ; j++ )
//       colorRemap->color_table[FIRST_REMAP_KEY+j] = (char) (remap_method_array[i].secondary_color+j);
   }
}
//---------- End of function Game::init_remap_table --------//


//--------- Begin of function Game::get_color_remap_table --------//
//
// <int> nationRecno  - 0 for independent nation
// <int> selectedFlag - whether display outline around the bitmap
//
char* Game::get_color_remap_table(int nationRecno, int selectedFlag)
{
   ColorRemap* colorRemap;
   char* colorRemapTable;

	// ###### begin Gilbert 1/10 ######//
   if( nationRecno==0 || nation_array.is_deleted(nationRecno) )    // independent units
	// ###### end Gilbert 1/10 ######//
      colorRemap = color_remap_array;
   else
      colorRemap = color_remap_array+nation_array[nationRecno]->color_scheme_id;

   colorRemapTable = colorRemap->color_table;

   //--------- set outline color ---------//

	#define FIRST_CYCLE_COLOR     0xEF
	#define CYCLE_COLOR_COUNT     1
   #define CYCLE_FRAME_INTERVAL  3

   if( selectedFlag )
   {
      int cycleId=0;
/*
      if( CYCLE_COLOR_COUNT==1 )
      {
         cycleId = 0;
      }
      else
      {
         cycleId = sys.frame_count / CYCLE_FRAME_INTERVAL % (CYCLE_COLOR_COUNT*2-2);

         if( cycleId >= CYCLE_COLOR_COUNT )              // cycle in reserved order
            cycleId = CYCLE_COLOR_COUNT*2-2 - cycleId;
      }
*/
      colorRemapTable[OUTLINE_CODE]        = FIRST_CYCLE_COLOR + cycleId;
      colorRemapTable[OUTLINE_SHADOW_CODE] = FIRST_CYCLE_COLOR + cycleId;
   }
   else
   {
      colorRemapTable[OUTLINE_CODE] = (char) TRANSPARENT_CODE;
      colorRemapTable[OUTLINE_SHADOW_CODE] = (char) SHADOW_CODE;
   }

   return colorRemapTable;
}
//--------- End of function Game::get_color_remap_table --------//
