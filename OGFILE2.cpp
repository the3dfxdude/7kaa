//Filename    : OGFILE2.CPP
//Description : Object Game file, save game and restore game, part 2

#include <OUNITRES.h>
#include <OFIRMRES.h>

#include <OUNIT.h>
#include <OBULLET.h>
#include <OSITE.h>
#include <OTOWN.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <OTECHRES.h>
#include <ORACERES.h>
#include <OTOWNRES.h>
#include <ORAWRES.h>
#include <OGODRES.h>
#include <OTALKRES.h>
#include <OTORNADO.h>
#include <OMONSRES.h>
#include <OREBEL.h>
#include <OSPY.h>
#include <OSNOWG.h>
#include <OSYS.h>
#include <OREGION.h>
#include <ONEWS.h>
#include <OTUTOR.h>
#include <OINFO.h>
#include <OWEATHER.h>
#include <OGAME.h>
#include <OPOWER.h>
#include <OWORLD.h>
#include <OGFILE.h>
#include <OROCK.h>
// ##### begin Gilbert 2/10 #######//
#include <OFIRMDIE.h>
// ##### end Gilbert 2/10 #######//


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

static int loaded_random_seed;

//-------- Define static class member vars -------//

File* GameFile::file_ptr;
char  GameFile::last_read_success_flag=0;

//-------- Begin of function GameFile::write_file -------//
//
// Save a game to file
//
// Return : 1 - file written successfully
//          0 - not successful
//
int GameFile::write_file(File* filePtr)
{
	file_ptr = filePtr;			// for write_book_mark()

	//----- check valid version first ------//

	if( game_file_array.demo_format )
		filePtr->file_put_short( -GAME_VERSION );    // negative no. means shareware version
	else
		filePtr->file_put_short( GAME_VERSION );

	//------------------------------------------------//
	//
	// The order of writing data is different between
	// the shareware and registered version.
	//
	//------------------------------------------------//

	if( game_file_array.demo_format )
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
	file_ptr = filePtr;			// for read_book_mark()

	//----- check version no. first ------//

	int originalRandomSeed = m.get_random_seed();

	//### begin alex 5/3 ###//
#ifdef AMPLUS
//	game_file_array.load_file_game_version = filePtr->file_get_short();
	game_file_array.load_file_game_version = filePtr->file_get_short();

	// compare if same demo format or not
	if( game_file_array.demo_format && game_file_array.load_file_game_version > 0
		|| !game_file_array.demo_format && game_file_array.load_file_game_version < 0)
		return -1;

	// take the absolute value of game version
	game_file_array.load_file_game_version = abs(game_file_array.load_file_game_version);

	if(game_file_array.load_file_game_version > GAME_VERSION)
		return -1;		// the executing program can't handle saved game in future version

//	game_file_array.same_version = (game_file_array.load_file_game_version/100==
//												(game_file_array.demo_format ? -(GAME_VERSION/100) : GAME_VERSION/100));
	game_file_array.same_version = ( game_file_array.load_file_game_version/100==GAME_VERSION/100 );
#else
	// ###### patch begin Gilbert 20/1 ########//
	// compare major version
	//if( filePtr->file_get_short() != (game_file_array.demo_format ? -GAME_VERSION : GAME_VERSION) )
	if( filePtr->file_get_short()/100 != (game_file_array.demo_format ? -(GAME_VERSION/100) : GAME_VERSION/100) )
		return -1;
	// ###### patch end Gilbert 20/1 ########//
#endif
	//#### end alex 5/3 ####//

	//------------------------------------------------//
	//
	// The order of writing data is different between
	// the shareware and registered version.
	//
	//------------------------------------------------//

	if( game_file_array.demo_format )
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

	err_when( originalRandomSeed != m.get_random_seed() );

	m.set_random_seed(loaded_random_seed);

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
	write_book_mark( BOOK_MARK+1 );

	if( !race_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+2 );

	if( !unit_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+3 );

	if( !firm_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+4 );

	if( !town_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+5 );

	if( !tech_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+6 );

	if( !talk_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+7 );

	if( !raw_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+8 );

	if( !god_res.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+9 );

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
	write_book_mark( BOOK_MARK+101 );

	if( !game.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+102 );

	if( !config.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+103 );

	if( !sys.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+104 );

	if( !info.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+105 );

	if( !power.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+106 );

	if( !weather.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+107 );

	if( !magic_weather.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+108 );

	if( !news_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+109 );

	if( !world.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+110 );

	if( !tutor.write_file(filePtr) )
		return 0;

	//### begin alex 23/9 ###//
	write_book_mark( BOOK_MARK+111 );

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
	write_book_mark( BOOK_MARK+201 );

	if( !unit_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+202 );

	if( !bullet_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+203 );

	if( !site_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+204 );

	if( !town_array.write_file(filePtr) )  // job will affect firm, group, item
		return 0;

	write_book_mark( BOOK_MARK+205 );

	if( !nation_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+206 );

	if( !firm_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+207 );

	if( !tornado_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+208 );

	if( !rebel_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+209 );

	if( !spy_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+210 );

	if( !snow_ground_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+211 );

	if( !region_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+212 );

	if( !news_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+213 );

	if( !rock_array.write_file(filePtr) )
		return 0;

	write_book_mark( BOOK_MARK+214 );

	if( !dirt_array.write_file(filePtr) )
		return 0;

	// ##### begin Gilbert 2/10 ######//
	write_book_mark( BOOK_MARK+215 );

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
	if( !read_book_mark( BOOK_MARK+1 ) )
		return 0;

	if( !race_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+2 ) )
		return 0;

	if( !unit_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+3 ) )
		return 0;

	if( !firm_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+4 ) )
		return 0;

	if( !town_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+5 ) )
		return 0;

	if( !tech_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+6 ) )
		return 0;

	if( !talk_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+7 ) )
		return 0;

	if( !raw_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+8 ) )
		return 0;

	if( !god_res.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+9 ) )
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
	if( !read_book_mark( BOOK_MARK+101 ) )
		return 0;

	if( !game.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+102 ) )
		return 0;

	if( !config.read_file(filePtr, 1) )		// 1-keep system settings
		return 0;

	if( !read_book_mark( BOOK_MARK+103 ) )
		return 0;

	if( !sys.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+104 ) )
		return 0;

	if( !info.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+105 ) )
		return 0;

	if( !power.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+106 ) )
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

	if( !read_book_mark( BOOK_MARK+107 ) )
		return 0;

	if( !magic_weather.read_file(filePtr) )
		return 0;

	sprite_res.update_speed();

	if( !read_book_mark( BOOK_MARK+108 ) )
		return 0;

	if( !news_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+109 ) )
		return 0;

	if( !world.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+110 ) )
		return 0;

	if( !tutor.read_file(filePtr) )
		return 0;

	//### begin alex 23/9 ###//
	if( !read_book_mark( BOOK_MARK+111 ) )
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
	if( !read_book_mark( BOOK_MARK+201 ) )
		return 0;

	if( !unit_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+202 ) )
		return 0;

	if( !bullet_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+203 ) )
		return 0;

	if( !site_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+204 ) )
		return 0;

	if( !town_array.read_file(filePtr) )  // job will affect firm, group, item
		return 0;

	if( !read_book_mark( BOOK_MARK+205 ) )
		return 0;

	if( !nation_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+206 ) )
		return 0;

	if( !firm_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+207 ) )
		return 0;

	if( !tornado_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+208 ) )
		return 0;

	if( !rebel_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+209 ) )
		return 0;

	if( !spy_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+210 ) )
		return 0;

	if( !snow_ground_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+211 ) )
		return 0;

	if( !region_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+212 ) )
		return 0;

	if( !news_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+213 ) )
		return 0;

	if( !rock_array.read_file(filePtr) )
		return 0;

	if( !read_book_mark( BOOK_MARK+214 ) )
		return 0;

	if( !dirt_array.read_file(filePtr) )
		return 0;

	// ##### begin Gilbert 2/10 ######//
	if( !read_book_mark( BOOK_MARK+215 ) )

	if( !firm_die_array.read_file(filePtr) )
		return 0;
	// ##### end Gilbert 2/10 ######//

	return 1;
}
//---------- End of function GameFile::read_file_3 -------//


//-------- Begin of function GameFile::write_book_mark -------//
//
void GameFile::write_book_mark(short bookMark)
{
	sys.yield();

	file_ptr->file_put_short(bookMark);
}
//---------- End of function GameFile::write_book_mark -------//


//-------- Begin of function GameFile::read_book_mark -------//
//
// Return : 1  - the book mark is matched
//          0  - the book mark is not matched
//
int GameFile::read_book_mark(short bookMark)
{
	sys.yield();

	return file_ptr->file_get_short() == bookMark;
}
//---------- End of function GameFile::read_book_mark -------//


//***//


//-------- Start of function RaceRes::write_file -------------//
//
int RaceRes::write_file(File* filePtr)
{
	//------- write RaceInfo -------//

	RaceInfo* raceInfo = race_info_array;

	for( int i=1 ; i<=race_res.race_count ; i++, raceInfo++ )
	{
		filePtr->file_put_short( raceInfo->town_name_used_count );
	}

	return filePtr->file_write( name_used_array, sizeof(name_used_array[0]) * name_count );
}
//--------- End of function RaceRes::write_file ---------------//


//-------- Start of function RaceRes::read_file -------------//
//
int RaceRes::read_file(File* filePtr)
{
	//------- read RaceInfo -------//

	RaceInfo* raceInfo = race_info_array;

	for( int i=1 ; i<=race_res.race_count ; i++, raceInfo++ )
	{
		#ifdef AMPLUS
		raceInfo->town_name_used_count = (!game_file_array.same_version && i>VERSION_1_MAX_RACE) ?
													0 : filePtr->file_get_short();
		#else
			raceInfo->town_name_used_count = filePtr->file_get_short();
		#endif
	}

#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		memset(name_used_array, 0, sizeof(name_used_array[0]) * name_count);
		return filePtr->file_read( name_used_array, sizeof(name_used_array[0]) * VERSION_1_RACERES_NAME_COUNT );
	}
	else
		return filePtr->file_read( name_used_array, sizeof(name_used_array[0]) * name_count );
#else
	return filePtr->file_read( name_used_array, sizeof(name_used_array[0]) * name_count );
#endif
}
//--------- End of function RaceRes::read_file ---------------//

//***//

//-------- Start of function UnitRes::write_file -------------//
//
int UnitRes::write_file(File* filePtr)
{
	filePtr->file_put_short(mobile_monster_count);

	UnitInfo* unitInfo = unit_info_array;

	for( int i=1 ; i<=unit_res.unit_info_count ; i++, unitInfo++ )
	{
		if( !filePtr->file_write( unitInfo->nation_tech_level_array, sizeof(unitInfo->nation_tech_level_array) ) )
			return 0;

		if( !filePtr->file_write( unitInfo->nation_unit_count_array, sizeof(unitInfo->nation_unit_count_array) ) )
			return 0;

		if( !filePtr->file_write( unitInfo->nation_general_count_array, sizeof(unitInfo->nation_general_count_array) ) )
			return 0;
	}

	return 1;
}
//--------- End of function UnitRes::write_file ---------------//


//-------- Start of function UnitRes::read_file -------------//
//
int UnitRes::read_file(File* filePtr)
{
	mobile_monster_count = filePtr->file_get_short();

	UnitInfo* unitInfo = unit_info_array;

	for( int i=1 ; i<=unit_res.unit_info_count ; i++, unitInfo++ )
	{
		#ifdef AMPLUS
			if(!game_file_array.same_version && i > VERSION_1_UNITRES_UNIT_INFO_COUNT)
			{
				memset(unitInfo->nation_tech_level_array, 0, sizeof(unitInfo->nation_tech_level_array));
				memset(unitInfo->nation_unit_count_array, 0, sizeof(unitInfo->nation_unit_count_array));
				memset(unitInfo->nation_general_count_array, 0, sizeof(unitInfo->nation_general_count_array));
				continue;
			}
		#endif

		if( !filePtr->file_read( unitInfo->nation_tech_level_array, sizeof(unitInfo->nation_tech_level_array) ) )
			return 0;

		if( !filePtr->file_read( unitInfo->nation_unit_count_array, sizeof(unitInfo->nation_unit_count_array) ) )
			return 0;

		if( !filePtr->file_read( unitInfo->nation_general_count_array, sizeof(unitInfo->nation_general_count_array) ) )
			return 0;
	}

	return 1;
}
//--------- End of function UnitRes::read_file ---------------//

//***//

//-------- Start of function FirmRes::write_file -------------//
//
int FirmRes::write_file(File* filePtr)
{
	return filePtr->file_write( firm_info_array, firm_count * sizeof(FirmInfo) );
}
//--------- End of function FirmRes::write_file ---------------//


//-------- Start of function FirmRes::read_file -------------//
//
int FirmRes::read_file(File* filePtr)
{
	int arraySize = firm_count * sizeof(FirmInfo);

	//----- save the firm names, so that it won't be overwritten by the saved game file ----//

	FirmInfo* oldFirmInfoArray = (FirmInfo*) mem_add(arraySize);

	memcpy( oldFirmInfoArray, firm_info_array, arraySize );

	int rc = filePtr->file_read( firm_info_array, arraySize );

	for( int i=0 ; i<firm_count ; i++ )
	{
		memcpy( firm_info_array[i].name			  , oldFirmInfoArray[i].name			  , FirmInfo::NAME_LEN+1 );
		memcpy( firm_info_array[i].short_name	  , oldFirmInfoArray[i].short_name	  , FirmInfo::SHORT_NAME_LEN+1 );
		memcpy( firm_info_array[i].overseer_title, oldFirmInfoArray[i].overseer_title, FirmInfo::TITLE_LEN+1 );
		memcpy( firm_info_array[i].worker_title  , oldFirmInfoArray[i].worker_title  , FirmInfo::TITLE_LEN+1 );

		// ###### patch begin Gilbert 11/3 ########//
		firm_info_array[i].first_build_id = oldFirmInfoArray[i].first_build_id;
		firm_info_array[i].build_count = oldFirmInfoArray[i].build_count;
		// ###### patch end Gilbert 11/3 ########//
	}

	mem_del( oldFirmInfoArray );

	return rc;
}
//--------- End of function FirmRes::read_file ---------------//

//***//

//-------- Start of function TownRes::write_file -------------//
//
int TownRes::write_file(File* filePtr)
{
	return filePtr->file_write( town_name_used_array, sizeof(town_name_used_array[0]) * town_name_count );
}
//--------- End of function TownRes::write_file ---------------//


//-------- Start of function TownRes::read_file -------------//
//
int TownRes::read_file(File* filePtr)
{
#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		memset(town_name_used_array, 0, sizeof(town_name_used_array));
		return filePtr->file_read( town_name_used_array, sizeof(town_name_used_array[0]) * VERSION_1_TOWNRES_TOWN_NAME_COUNT );
	}
	else
		return filePtr->file_read( town_name_used_array, sizeof(town_name_used_array[0]) * town_name_count );
#else
	return filePtr->file_read( town_name_used_array, sizeof(town_name_used_array[0]) * town_name_count );
#endif
}
//--------- End of function TownRes::read_file ---------------//

//***//

//-------- Start of function TechRes::write_file -------------//
//
int TechRes::write_file(File* filePtr)
{
	if( !filePtr->file_write( tech_class_array, tech_class_count * sizeof(TechClass) ) )
		return 0;

	if( !filePtr->file_write( tech_info_array, tech_count * sizeof(TechInfo) ) )
		return 0;

	return 1;
}
//--------- End of function TechRes::write_file ---------------//


//-------- Start of function TechRes::read_file -------------//
//
int TechRes::read_file(File* filePtr)
{
	if( !filePtr->file_read( tech_class_array, tech_class_count * sizeof(TechClass) ) )
		return 0;

#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		if(!filePtr->file_read( tech_info_array, VERSION_1_TECH_COUNT * sizeof(TechInfo) ) )
			return 0;

		TechInfo *techInfoPtr = tech_info_array + VERSION_1_TECH_COUNT;
		for(int i=VERSION_1_TECH_COUNT; i<tech_count; ++i, techInfoPtr++)
		{
			memset(techInfoPtr->nation_tech_level_array, 0, sizeof(techInfoPtr->nation_tech_level_array));
			memset(techInfoPtr->nation_is_researching_array, 0, sizeof(techInfoPtr->nation_is_researching_array));
			memset(techInfoPtr->nation_research_progress_array, 0, sizeof(techInfoPtr->nation_research_progress_array));
		}
	}
	else
	{
		if( !filePtr->file_read( tech_info_array, tech_count * sizeof(TechInfo) ) )
			return 0;
	}
#else
	if( !filePtr->file_read( tech_info_array, tech_count * sizeof(TechInfo) ) )
		return 0;
#endif

	return 1;
}
//--------- End of function TechRes::read_file ---------------//

//***//

//-------- Start of function TalkRes::write_file -------------//
//
int TalkRes::write_file(File* filePtr)
{
	if( !filePtr->file_write( this, sizeof(TalkRes) ) )
		return 0;

	if( !talk_msg_array.write_file(filePtr) )
		return 0;

	return 1;
}
//--------- End of function TalkRes::write_file ---------------//


//-------- Start of function TalkRes::read_file -------------//
//
int TalkRes::read_file(File* filePtr)
{
	//------ save talk_msg_array --------//

	char* tempArray = mem_add(sizeof(DynArrayB));

	memcpy( tempArray, &talk_msg_array, sizeof(DynArrayB) );

	//------ read in TalkRes --------//

	if( !filePtr->file_read( this, sizeof(TalkRes) ) )
		return 0;

	//------ restore talk_msg_array --------//

	memcpy( &talk_msg_array, tempArray, sizeof(DynArrayB) );

	mem_del(tempArray);

	//---- read in talk_msg_array ----//

	if( !talk_msg_array.read_file(filePtr) )
		return 0;

	return 1;
}
//--------- End of function TalkRes::read_file ---------------//

//***//

//-------- Start of function RawRes::write_file -------------//
//
int RawRes::write_file(File* filePtr)
{
	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		if( !raw_info_array[i].raw_supply_firm_array.write_file(filePtr) )
			return 0;

		if( !raw_info_array[i].product_supply_firm_array.write_file(filePtr) )
			return 0;
	}

	return 1;
}
//--------- End of function RawRes::write_file ---------------//


//-------- Start of function RawRes::read_file -------------//
//
int RawRes::read_file(File* filePtr)
{
	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		if( !raw_info_array[i].raw_supply_firm_array.read_file(filePtr) )
			return 0;

		if( !raw_info_array[i].product_supply_firm_array.read_file(filePtr) )
			return 0;
	}

	return 1;
}
//--------- End of function RawRes::read_file ---------------//

//***//

//-------- Start of function GodRes::write_file -------------//
//
int GodRes::write_file(File* filePtr)
{
	return filePtr->file_write( god_info_array, sizeof(GodInfo) * god_count );
}
//--------- End of function GodRes::write_file ---------------//


//-------- Start of function GodRes::read_file -------------//
//
int GodRes::read_file(File* filePtr)
{
#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		memset(god_info_array, 0, sizeof(god_info_array));
		return filePtr->file_read( god_info_array, sizeof(GodInfo) * VERSION_1_GODRES_GOD_COUNT );
	}
	else
		return filePtr->file_read( god_info_array, sizeof(GodInfo) * god_count );
#else
	return filePtr->file_read( god_info_array, sizeof(GodInfo) * god_count );
#endif
}
//--------- End of function GodRes::read_file ---------------//

//***//

//-------- Start of function MonsterRes::write_file -------------//
//
int MonsterRes::write_file(File* filePtr)
{
	return filePtr->file_write( active_monster_array, sizeof(active_monster_array) );
}
//--------- End of function MonsterRes::write_file ---------------//


//-------- Start of function MonsterRes::read_file -------------//
//
int MonsterRes::read_file(File* filePtr)
{
	return filePtr->file_read( active_monster_array, sizeof(active_monster_array) );
}
//--------- End of function MonsterRes::read_file ---------------//

//***//

//-------- Start of function Game::write_file -------------//
//
int Game::write_file(File* filePtr)
{
	return filePtr->file_write( this, sizeof(Game) );
}
//--------- End of function Game::write_file ---------------//


//-------- Start of function Game::read_file -------------//
//
int Game::read_file(File* filePtr)
{
	return filePtr->file_read( this, sizeof(Game) );
}
//--------- End of function Game::read_file ---------------//

//***//

//-------- Start of function Config::write_file -------------//
//
int Config::write_file(File* filePtr)
{
	return filePtr->file_write( this, sizeof(Config) );
}
//--------- End of function Config::write_file ---------------//


//-------- Start of function Config::read_file -------------//
//
int Config::read_file(File* filePtr, int keepSysSettings)
{
	//--- these settings are not game dependent -----//

	char  musicFlag 		 = music_flag;
	short cdMusicVol  	 = cd_music_volume;
	short	wavMusicVol 	 = wav_music_volume;
	char	soundEffectFlag = sound_effect_flag;
	short	soundEffectVol  = sound_effect_volume;
	char	helpMode			 = help_mode;

	int rc = filePtr->file_read( this, sizeof(Config) );

	if( keepSysSettings )
	{
		music_flag		   = musicFlag;
		cd_music_volume   = cdMusicVol;
		wav_music_volume  = wavMusicVol;
		sound_effect_flag = soundEffectFlag;
		sound_effect_volume = soundEffectVol;
		help_mode			= helpMode;
	}

	return rc;
}
//--------- End of function Config::read_file ---------------//

//***//

//-------- Start of function Info::write_file -------------//
//
int Info::write_file(File* filePtr)
{
	int writeSize = (char*)(&last_write_offset) - (char*)(this);

	//---------- write the info data -----------//

	return filePtr->file_write( this, writeSize );
}
//--------- End of function Info::write_file ---------------//


//-------- Start of function Info::read_file -------------//
//
int Info::read_file(File* filePtr)
{
	int readSize = (char*)(&last_write_offset) - (char*)(this);

	//------- read the info data ----------//

	return filePtr->file_read( this, readSize );
}
//--------- End of function Info::read_file ---------------//

//***//

//-------- Start of function Power::write_file -------------//
//
int Power::write_file(File* filePtr)
{
	return filePtr->file_write( this, sizeof(Power) );
}
//--------- End of function Power::write_file ---------------//


//-------- Start of function Power::read_file -------------//
//
int Power::read_file(File* filePtr)
{
	return filePtr->file_read( this, sizeof(Power) );
}
//--------- End of function Power::read_file ---------------//

//***//

//-------- Start of function Sys::write_file -------------//
//
int Sys::write_file(File* filePtr)
{
	//---- write the current random seed first ----//

	if( !filePtr->file_put_long(m.get_random_seed()) )
		return 0;

	//---------- write some Sys data -----------//

	filePtr->file_put_long(day_frame_count);
	filePtr->file_put_long(frame_count);
	filePtr->file_put_short(view_mode);

	return 1;
}
//--------- End of function Sys::write_file ---------------//


//-------- Start of function Sys::read_file -------------//
//
int Sys::read_file(File* filePtr)
{
	//------- read the random seed --------//

	loaded_random_seed = filePtr->file_get_long();

	//--------- read some Sys data -----------//

	day_frame_count = filePtr->file_get_long();
	frame_count 	 = filePtr->file_get_long();
	view_mode       = (char) filePtr->file_get_short();

	return 1;
}
//--------- End of function Sys::read_file ---------------//

//***//

//-------- Start of function Weather::write_file -------------//
//
int Weather::write_file(File* filePtr)
{
	return filePtr->file_write( this, sizeof(Weather) );
}
//--------- End of function Weather::write_file ---------------//


//-------- Start of function Weather::read_file -------------//
//
int Weather::read_file(File* filePtr)
{
	return filePtr->file_read( this, sizeof(Weather) );
}
//--------- End of function Weather::read_file ---------------//

//***//

//-------- Start of function MagicWeather::write_file -------------//
//
int MagicWeather::write_file(File* filePtr)
{
	return filePtr->file_write( this, sizeof(MagicWeather) );
}
//--------- End of function MagicWeahter::write_file ---------------//


//-------- Start of function MagicWeahter::read_file -------------//
//
int MagicWeather::read_file(File* filePtr)
{
	return filePtr->file_read( this, sizeof(MagicWeather) );
}
//--------- End of function MagicWeahter::read_file ---------------//

//***//

//-------- Start of function World::write_file -------------//
//
int World::write_file(File* filePtr)
{
	//--------- save map -------------//

	if( !filePtr->file_write(loc_matrix, max_x_loc*max_y_loc*sizeof(Location) ) )
		return 0;

	//--------- save vars -----------//

	filePtr->file_put_short(scan_fire_x);
	filePtr->file_put_short(scan_fire_y);
	filePtr->file_put_short(lightning_signal);
	// ######## begin Gilbert 18/7 #########//
	filePtr->file_put_long(plant_count);
	filePtr->file_put_long(plant_limit);
	// ######## end Gilbert 18/7 #########//

	map_matrix->last_map_mode = -1;

	filePtr->file_put_short(map_matrix->map_mode);
	filePtr->file_put_short(map_matrix->power_mode);

	filePtr->file_put_long(map_matrix->cur_x_loc);
	filePtr->file_put_long(map_matrix->cur_y_loc);

	filePtr->file_put_long(zoom_matrix->init_lightning);
	filePtr->file_put_long(zoom_matrix->vibration);
	filePtr->file_put_short(zoom_matrix->lightning_x1);
	filePtr->file_put_short(zoom_matrix->lightning_y1);
	filePtr->file_put_short(zoom_matrix->lightning_x2);
	filePtr->file_put_short(zoom_matrix->lightning_y2);

	return 1;
}
//--------- End of function World::write_file ---------------//


//-------- Start of function World::read_file -------------//
//
int World::read_file(File* filePtr)
{
	//-------- read in the map --------//

	loc_matrix = (Location*) mem_resize( loc_matrix, max_x_loc * max_y_loc
					  * sizeof(Location) );

	if( !filePtr->file_read(loc_matrix, max_x_loc*max_y_loc*sizeof(Location) ) )
		return 0;

	assign_map();

	//--------- read in vars ----------//

	scan_fire_x 	  = (char) filePtr->file_get_short();
	scan_fire_y 	  = (char) filePtr->file_get_short();
	lightning_signal = (char) filePtr->file_get_short();
	// ######## begin Gilbert 18/7 #########//
	plant_count      = filePtr->file_get_long();
	plant_limit      = filePtr->file_get_long();
	// ######## end Gilbert 18/7 #########//

	map_matrix->last_map_mode = -1;

	map_matrix->map_mode   = (char) filePtr->file_get_short();
	map_matrix->power_mode = (char) filePtr->file_get_short();

	map_matrix->cur_x_loc = filePtr->file_get_long();
	map_matrix->cur_y_loc = filePtr->file_get_long();

	zoom_matrix->top_x_loc = map_matrix->cur_x_loc;
	zoom_matrix->top_y_loc = map_matrix->cur_y_loc;

	sys.zoom_need_redraw = 1;

	zoom_matrix->init_lightning = filePtr->file_get_long();
	zoom_matrix->vibration = filePtr->file_get_long();
	zoom_matrix->lightning_x1 = filePtr->file_get_short();
	zoom_matrix->lightning_y1 = filePtr->file_get_short();
	zoom_matrix->lightning_x2 = filePtr->file_get_short();
	zoom_matrix->lightning_y2 = filePtr->file_get_short();

	return 1;
}
//--------- End of function World::read_file ---------------//

//***//

//-------- Start of function Tutor::write_file -------------//
//
int Tutor::write_file(File* filePtr)
{
	filePtr->file_put_short(cur_tutor_id);
	filePtr->file_put_short(cur_text_block_id);

	return 1;
}
//--------- End of function Tutor::write_file ---------------//


//-------- Start of function Tutor::read_file -------------//
//
int Tutor::read_file(File* filePtr)
{
	int curTutorId =	filePtr->file_get_short();

	if( curTutorId > 0 )
		tutor.load(curTutorId);		// load() will reset cur_text_block_id

	cur_text_block_id	= filePtr->file_get_short();
	last_text_block_id = 0;

	return 1;
}
//--------- End of function Tutor::read_file ---------------//

//### begin alex 23/9 ###//
//-------- Start of function SeekPath::write_file -------------//
//
int SeekPath::write_file(File* filePtr)
{
	filePtr->file_put_short(total_node_avail);
	return 1;
}
//--------- End of function SeekPath::write_file ---------------//


//-------- Start of function SeekPath::read_file -------------//
//
int SeekPath::read_file(File* filePtr)
{
	total_node_avail =	filePtr->file_get_short();
	return 1;
}
//--------- End of function SeekPath::read_file ---------------//
//#### end alex 23/9 ####//
