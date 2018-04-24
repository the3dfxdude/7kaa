/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
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

//Filename    : OGFILE2.CPP
//Description : Object Game file, save game and restore game, part 2

#include <OUNITRES.h>
#include <OFIRMRES.h>

#include <OBULLET.h>
#include <OFIRM.h>
#include <OFIRMDIE.h>
#include <OGAME.h>
#include <OGFILE.h>
#include <OGF_V1.h>
#include <OGODRES.h>
#include <OINFO.h>
#include <OMONSRES.h>
#include <ONATION.h>
#include <ONEWS.h>
#include <OPOWER.h>
#include <ORACERES.h>
#include <ORAWRES.h>
#include <OREBEL.h>
#include <OREGION.h>
#include <OROCK.h>
#include <OSITE.h>
#include <OSNOWG.h>
#include <OSPY.h>
#include <OSYS.h>
#include <OTALKRES.h>
#include <OTECHRES.h>
#include <OTORNADO.h>
#include <OTOWN.h>
#include <OTOWNRES.h>
#include <OTUTOR.h>
#include <OUNIT.h>
#include <OWEATHER.h>
#include <OWORLD.h>
#include <OSaveGameArray.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(GameFile);


//-------- Define constant ----------//

#define BOOK_MARK 0x1000         // book mark for validing saving data


//----------- Game file format --------------//
//
// Data of the following objects will be saved :
//
// RaceRes				race_res;
// UnitRes				unit_res;
// FirmRes				firm_res;
// TownRes				town_res;
// TechRes				tech_res;
// TalkRes				talk_res;
// RawRes				raw_res;
// GodRes				god_res;
// MonsterRes			monster_res;
//
// UnitArray			unit_array(100); 			// with derived classes
// BulletArray			bullet_array(100);
// SiteArray			site_array;
// TownArray			town_array;
// NationArray			nation_array;
// FirmArray			firm_array;					// with derived classes
// TornadoArray		tornado_array(10);
// RebelArray			rebel_array;
// SpyArray				spy_array;
// SnowGroundArray	snow_ground_array;
// RegionArray			region_array;
// NewsArray			news_array;
//
// Game					game
//	Config				config
// Sys				   sys
// Info					info
// Power					power
// Weather				weather, weather_forecast[MAX_WEATHER_FORECAST];
// MagicWeather		magic_weather
// News					news
// World					world
// Tutor					tutor
//### begin alex 23/9 ###//
// SeekPath				seek_path
//#### end alex 23/9 ####//
//
//-------------------------------------------//

//--------- Define static vars ---------//

bool GameFile::read_file_same_version = true;

//-------- Begin of function GameFile::write_file -------//
//
// Save a game to file
//
// Return : 1 - file written successfully
//          0 - not successful
//
int GameFile::write_file(File* filePtr)
{
	bool demo_format = false;
#if defined(DEMO) || defined(DEMO_DESIGN)
	demo_format = true;
#endif

	//----- check valid version first ------//

	if( demo_format )
		filePtr->file_put_short( -GAME_VERSION );    // negative no. means shareware version
	else
		filePtr->file_put_short( GAME_VERSION );

	//------------------------------------------------//
	//
	// The order of writing data is different between
	// the shareware and registered version.
	//
	//------------------------------------------------//

	if( demo_format )
	{
		if( !write_file_1(filePtr) )
			return 0;

		if( !write_file_2(filePtr) )
			return 0;
	}
	else
	{
		if( !write_file_2(filePtr) )
			return 0;

		if( !write_file_1(filePtr) )
         return 0;
   }

   if( !write_file_3(filePtr) )
		return 0;

   return 1;
}
//---------- End of function GameFile::write_file -------//


//-------- Begin of function GameFile::read_file -------//
//
// Restore a game from file
//
// Return : 1  - file read successfully
//          0  - not successful
//          -1 - incorrect game data version error
//
int GameFile::read_file(File* filePtr)
{
	bool demo_format = false;
#if defined(DEMO) || defined(DEMO_DESIGN)
	demo_format = true;
#endif

	//----- check version no. first ------//

	int originalRandomSeed = misc.get_random_seed();

	short load_file_game_version = filePtr->file_get_short();

	// compare if same demo format or not
	if( demo_format && load_file_game_version > 0
		|| !demo_format && load_file_game_version < 0)
		return -1;

	// take the absolute value of game version
	load_file_game_version = abs(load_file_game_version);

	if(load_file_game_version > GAME_VERSION)
		return -1;		// the executing program can't handle saved game in future version

	read_file_same_version = ( load_file_game_version/100==GAME_VERSION/100 );

	//------------------------------------------------//
	//
	// The order of writing data is different between
	// the shareware and registered version.
	//
	//------------------------------------------------//

	if( demo_format )
	{
		if( !read_file_1(filePtr) )
			return 0;

		if( !read_file_2(filePtr) )
			return 0;
	}
	else
	{
		if( !read_file_2(filePtr) )
			return 0;

		if( !read_file_1(filePtr) )
			return 0;
	}

	if( !read_file_3(filePtr) )
		return 0;

	//-------------------------------------//

	err_when( originalRandomSeed != misc.get_random_seed() );

	misc.set_random_seed(sys.loaded_random_seed);

	return 1;
}
//---------- End of function GameFile::read_file -------//


//-------- Begin of function GameFile::write_file_1 -------//
//
// Return : 1 - file written successfully
//          0 - not successful
//
int GameFile::write_file_1(File* filePtr)
{
	write_book_mark( filePtr, BOOK_MARK+1 );

	if( !race_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+2 );

	if( !unit_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+3 );

	if( !firm_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+4 );

	if( !town_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+5 );

	if( !tech_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+6 );

	if( !talk_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+7 );

	if( !raw_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+8 );

	if( !god_res.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+9 );

	if( !monster_res.write_file(filePtr) )
		return 0;

	return 1;
}
//---------- End of function GameFile::write_file_1 -------//


//-------- Begin of function GameFile::write_file_2 -------//
//
// Save a game to file
//
// Return : 1 - file written successfully
//          0 - not successful
//
int GameFile::write_file_2(File* filePtr)
{
	write_book_mark( filePtr, BOOK_MARK+101 );

	if( !game.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+102 );

	if( !config.write_file(filePtr, false /* don't include system settings */) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+103 );

	if( !sys.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+104 );

	if( !info.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+105 );

	if( !power.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+106 );

	if( !weather.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+107 );

	if( !magic_weather.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+108 );

	if( !news_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+109 );

	if( !world.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+110 );

	if( !tutor.write_file(filePtr) )
		return 0;

	//### begin alex 23/9 ###//
	write_book_mark( filePtr, BOOK_MARK+111 );

	if( !seek_path.write_file(filePtr) )
		return 0;
	//#### end alex 23/9 ####//

	return 1;
}
//---------- End of function GameFile::write_file_2 -------//


//-------- Begin of function GameFile::write_file_3 -------//
//
// Return : 1 - file written successfully
//          0 - not successful
//
int GameFile::write_file_3(File* filePtr)
{
	write_book_mark( filePtr, BOOK_MARK+201 );

	if( !unit_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+202 );

	if( !bullet_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+203 );

	if( !site_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+204 );

	if( !town_array.write_file(filePtr) )  // job will affect firm, group, item
		return 0;

	write_book_mark( filePtr, BOOK_MARK+205 );

	if( !nation_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+206 );

	if( !firm_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+207 );

	if( !tornado_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+208 );

	if( !rebel_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+209 );

	if( !spy_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+210 );

	if( !snow_ground_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+211 );

	if( !region_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+212 );

	if( !news_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+213 );

	if( !rock_array.write_file(filePtr) )
		return 0;

	write_book_mark( filePtr, BOOK_MARK+214 );

	if( !dirt_array.write_file(filePtr) )
		return 0;

	// ##### begin Gilbert 2/10 ######//
	write_book_mark( filePtr, BOOK_MARK+215 );

	if( !firm_die_array.write_file(filePtr) )
		return 0;
	// ##### end Gilbert 2/10 ######//

	return 1;
}
//---------- End of function GameFile::write_file_3 -------//


//-------- Begin of function GameFile::read_file_1 -------//
//
// Return : 1  - file read successfully
//          0  - not successful
//
int GameFile::read_file_1(File* filePtr)
{
	if( !read_book_mark( filePtr, BOOK_MARK+1 ) )
		return 0;

	if( !race_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+2 ) )
		return 0;

	if( !unit_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+3 ) )
		return 0;

	if( !firm_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+4 ) )
		return 0;

	if( !town_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+5 ) )
		return 0;

	if( !tech_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+6 ) )
		return 0;

	if( !talk_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+7 ) )
		return 0;

	if( !raw_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+8 ) )
		return 0;

	if( !god_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+9 ) )
		return 0;

	if( !monster_res.read_file(filePtr) )
		return 0;

	return 1;
}
//---------- End of function GameFile::read_file_1 -------//



//-------- Begin of function GameFile::read_file_2 -------//
//
// Return : 1  - file read successfully
//          0  - not successful
//
int GameFile::read_file_2(File* filePtr)
{
	if( !read_book_mark( filePtr, BOOK_MARK+101 ) )
		return 0;

	if( !game.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+102 ) )
		return 0;

	if( !config.read_file(filePtr, false /* don't include system settings */) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+103 ) )
		return 0;

	if( !sys.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+104 ) )
		return 0;

	if( !info.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+105 ) )
		return 0;

	if( !power.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+106 ) )
		return 0;

	if( !weather.read_file(filePtr) )
		return 0;

	weather_forecast[0] = weather;
	weather_forecast[0].next_day();

	for(int foreDay=1; foreDay < MAX_WEATHER_FORECAST; ++foreDay)
	{
		weather_forecast[foreDay] = weather_forecast[foreDay-1];
		weather_forecast[foreDay].next_day();
	}

	if( !read_book_mark( filePtr, BOOK_MARK+107 ) )
		return 0;

	if( !magic_weather.read_file(filePtr) )
		return 0;

	sprite_res.update_speed();

	if( !read_book_mark( filePtr, BOOK_MARK+108 ) )
		return 0;

	if( !news_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+109 ) )
		return 0;

	if( !world.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+110 ) )
		return 0;

	if( !tutor.read_file(filePtr) )
		return 0;

	//### begin alex 23/9 ###//
	if( !read_book_mark( filePtr, BOOK_MARK+111 ) )
		return 0;

	if( !seek_path.read_file(filePtr) )
		return 0;
	//#### end alex 23/9 ####//

	return 1;
}
//---------- End of function GameFile::read_file_2 -------//


//-------- Begin of function GameFile::read_file_3 -------//
//
// Return : 1  - file read successfully
//          0  - not successful
//
int GameFile::read_file_3(File* filePtr)
{
	if( !read_book_mark( filePtr, BOOK_MARK+201 ) )
		return 0;

	if( !unit_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+202 ) )
		return 0;

	if( !bullet_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+203 ) )
		return 0;

	if( !site_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+204 ) )
		return 0;

	if( !town_array.read_file(filePtr) )  // job will affect firm, group, item
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+205 ) )
		return 0;

	if( !nation_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+206 ) )
		return 0;

	if( !firm_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+207 ) )
		return 0;

	if( !tornado_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+208 ) )
		return 0;

	if( !rebel_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+209 ) )
		return 0;

	if( !spy_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+210 ) )
		return 0;

	if( !snow_ground_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+211 ) )
		return 0;

	if( !region_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+212 ) )
		return 0;

	if( !news_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+213 ) )
		return 0;

	if( !rock_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( filePtr, BOOK_MARK+214 ) )
		return 0;

	if( !dirt_array.read_file(filePtr) )
		return 0;

	// ##### begin Gilbert 2/10 ######//
	if( !read_book_mark( filePtr, BOOK_MARK+215 ) )
		return 0;

	if( !firm_die_array.read_file(filePtr) )
		return 0;
	// ##### end Gilbert 2/10 ######//

	return 1;
}
//---------- End of function GameFile::read_file_3 -------//


//-------- Begin of function GameFile::write_book_mark -------//
//
void GameFile::write_book_mark(File* filePtr, short bookMark)
{
	sys.yield();

	filePtr->file_put_short(bookMark);
}
//---------- End of function GameFile::write_book_mark -------//


//-------- Begin of function GameFile::read_book_mark -------//
//
// Return : 1  - the book mark is matched
//          0  - the book mark is not matched
//
int GameFile::read_book_mark(File* filePtr, short bookMark)
{
	sys.yield();

	return filePtr->file_get_short() == bookMark;
}
//---------- End of function GameFile::read_book_mark -------//
