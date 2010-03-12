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

//Filename    : OGFILE.CPP
//Description : Object Game file, save game and restore game

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <OBOX.h>
#include <OSTR.h>
#include <ODATE.h>
#include <OMOUSECR.h>
#include <OTALKRES.h>
#include <ONATION.h>
#include <OWORLD.h>
#include <OPOWER.h>
#include <OGAME.h>
#include <OINFO.h>
#include <OGFILE.h>
#include <OSYS.h>
#include <OAUDIO.h>
#include <OMUSIC.h>

// -------- define constant ----------//
#define MIN_FREE_SPACE 1000

//-------- Begin of function GameFile::save_game --------//
//
// return : <int> 1 - saved successfully.
//                0 - not saved.
//
int GameFile::save_game(const char* fileName)
{
	File   file;
	String errStr;

	power.win_opened=1;				// to disable power.mouse_handler()

	if( fileName )
		strcpy( file_name, fileName );

	int rc = 1;
	char lowDiskSpaceFlag = 0;
	DWORD sectorPerCluster = 0;
	DWORD bytePerSector = 0;
	DWORD freeCluster = 0;
	DWORD totalCluster = 0;
	if( GetDiskFreeSpace( NULL,	// address of root path, NULL means the current root directory
		&sectorPerCluster, &bytePerSector, &freeCluster, &totalCluster))
	{
		DWORD freeSpace = DWORD( (double)freeCluster * sectorPerCluster * bytePerSector / 1024.0);

		if( m.is_file_exist(file_name) )
		{	
			// if overwritting existing file, count the file size of the existing file
			file.file_open(file_name);
			freeSpace += file.file_size() / 1024;		// count the existing space
			file.file_close();
		}
		if( !(rc = freeSpace >= MIN_FREE_SPACE) )
		{
			errStr = "Insufficient disk space ! The game is not saved.";
			lowDiskSpaceFlag = 1;
		}
	}

	if( rc )
	{
		rc = file.file_create( file_name, 0, 1 );	// 0=tell File don't handle error itself
																   // 1=allow the writing size and the read size to be different
		if( !rc )
			errStr = "Error creating saved game file.";
	}

	if( rc )
	{
		save_process();      // process game data before saving the game

		rc = write_game_header(&file);    // write saved game header information

		if( !rc )
			errStr = "Error creating saved game header.";

		if( rc )
		{
			rc = write_file(&file);

			if( !rc )
				errStr = "Error writing saved game data.";
		}
	}

	file.file_close();

	power.win_opened=0;

	//------- when saving error ---------//

	if( !rc )
	{
		if( !lowDiskSpaceFlag )
			remove( file_name );         // delete the file as it is not complete

		#ifndef DEBUG
			errStr = "Insufficient disk space ! The game is not saved.";		// use this message for all types of error message in the release version
		#endif

		box.msg( errStr );
	}

	return rc;
}
//--------- End of function GameFile::save_game --------//


//-------- Begin of function GameFile::load_game --------//
//
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int GameFile::load_game(char* fileName)
{
	File file;
	int  rc=0;
	const char *errMsg = NULL;

	power.win_opened=1;				// to disable power.mouse_handler()

	int oldCursor = mouse_cursor.get_icon();
	mouse_cursor.set_icon( CURSOR_WAITING );

	int powerEnableFlag = power.enable_flag;

	if( fileName )
		strcpy( file_name, fileName );

	rc = 1;
	if( !file.file_open( file_name, 0, 1 ) )   // 0=tell File don't handle error itself
	{
		rc = 0;
		errMsg = "Cannot open save game file";
	}

	//-------- read in the GameFile class --------//

   if( rc )
   {
		char gameFileName[MAX_PATH+1];

      strcpy( gameFileName, file_name );     // save the file name actually read, in case that the file names are different

		if( !file.file_read(this, sizeof(GameFile)) )	// read the whole object from the saved game file
		{
			rc = 0;
			errMsg = "Cannot read file header";
		}
		if( rc )
		{
			if( !validate_header() )
			{
				rc = 0;
				errMsg = "Save game incompatible";
			}
			else
				strcpy( file_name, gameFileName );
		}
	}

	//--------------------------------------------//
																  // 1=allow the writing size and the read size to be different
	if( rc )
	{
		config.terrain_set = terrain_set;

		game.deinit(1);		// deinit last game first, 1-it is called during loading of a game
		game.init(1);			// init game

		//-------- read in saved game ----------//

		// ###### patch begin Gilbert 20/1 #######//
		//if( !read_file(&file) )
		//{
		//	rc = -1;
		//	errMsg = "Load game error";
		//}

		switch( read_file(&file) )
		{
		case 1:
			rc = 1;
			break;
		case -1:
			rc = 0;		// consider cancel load game
			errMsg = "Incompatible save game";
			break;
		case 0:
		default:
			rc = -1;
			errMsg = "Load game error";
		}

		if( rc > 0 )
			load_process();           // process game data after loading the game
		// ###### patch end Gilbert 20/1 #######//
	}

	file.file_close();

	power.enable_flag = powerEnableFlag;

	mouse_cursor.restore_icon( oldCursor );

	power.win_opened=0;

	//---------------------------------------//

	switch(rc)   // don't display msg if loaded successfully (rc==1)
	{
		case 0:
		case -1:
			box.msg( errMsg );
			break;
	}

   last_read_success_flag = rc;		// for external functions to read.  

	return rc;
}
//--------- End of function GameFile::load_game --------//


//------- Begin of function GameFile::set_file_name -------//
//
// Set the game file name of current save game, called by
// GameFile::save_game().
//
// e.g. ENLI_001.SAV - the first saved game of the group "Enlight Enterprise"
//
void GameFile::set_file_name()
{
	enum { NAME_PREFIX_LEN = 4,    // Maximum 4 characters in name prefix, e.g. "ENLIG" for "Enlight Enterprise"
			 NAME_NUMBER_LEN = 3  };

	String str, str2;
	int    i;
	char   nameChar;
	char*  baseName;             // the long name which the file name is based on
	char   addStr[] = "0";       // as a small string for adding to the large string

	baseName = (~nation_array)->king_name();

	//--------- add the group name prfix ----------//

	for( i=0 ; i<(int) strlen(baseName) && (int) str.len() < NAME_PREFIX_LEN ; i++ )
	{
		nameChar = m.upper(baseName[i]);

		if( ( nameChar >= 'A' && nameChar <= 'Z' ) ||
			 ( nameChar >= '0' && nameChar <= '9' ) )
		{
			addStr[0] = nameChar;

			str += addStr;
		}
	}

	//----- add tailing characters if prefix len < NAME_PREFIX_LEN+1 ---//

	while( str.len() < NAME_PREFIX_LEN+1 )       // +1 is the "_" between the name and the number
		str += "_";

	//---- find the saved game number for this saved game ----//

	int       curNumber, lastNumber=0;
	GameFile* gameFile;

	for( i=1 ; i<=game_file_array.size() ; i++ )
	{
		gameFile = game_file_array[i];

		// ##### begin Gilbert 3/10 ########//
		// if( memcmp(gameFile->file_name, str, NAME_PREFIX_LEN)==0 )
		if( strnicmp(gameFile->file_name, str, NAME_PREFIX_LEN)==0 )
		// ##### end Gilbert 3/10 ########//
		{
			//------------------------------------------------//
			//
			// if there is a free number in the middle of the list
			// (left by being deleted game), use this number.
			//
			//------------------------------------------------//

			curNumber = atoi( gameFile->file_name+NAME_PREFIX_LEN+1 );      // +1 is to pass the "_" between the name and the number

			if( curNumber > lastNumber+1 )   // normally, curNumber should be lastNumber+1
				break;

			lastNumber = curNumber;
		}
	}

	//------- add saved game number after the prefix --------//

	str2 = lastNumber+1;    // use the next number after the last number

	for( i=NAME_NUMBER_LEN-str2.len() ; i>0 ; i-- )   // add "0" before the number if the len of the number < NAME_NUMBER_LEN
		str += "0";

	str += str2;
	str += ".SAV";

	//----- copy the string to file_name ------//

	strncpy( file_name, str, MAX_PATH );

	file_name[MAX_PATH] = NULL;
}
//--------- End of function GameFile::set_file_name -------//


//-------- Begin of function GameFile::save_process -------//
//
// Make the game data ready for saving game
//
// Called before saving the game
//
void GameFile::save_process()
{
   //--------- set the total playing time --------//

	info.total_play_time += m.get_time()-info.start_play_time;

   info.start_play_time  = m.get_time();
}
//--------- End of function GameFile::save_process -------//


//-------- Begin of function GameFile::load_process -------//
//
// Make the game data ready after loading game
//
// Called after loading the game
//
void GameFile::load_process()
{
	info.start_play_time = m.get_time();       // the time player start playing the game
	config.disable_ai_flag = 0;

	//-- if the player is in the diplomatic message screen, rebuild the talk choice list --//

	if( sys.view_mode==MODE_NATION && info.nation_report_mode==NATION_REPORT_TALK )
		talk_res.set_talk_choices();

	mouse_cursor.set_frame(0);		// to fix a frame bug with loading game

	// reflect the effect of config.music_flag, config.wav_music_volume
	audio.set_wav_volume(config.sound_effect_volume);
	if( config.music_flag )
	{
		if( music.is_playing() )
		{
			music.change_volume(config.wav_music_volume);
		}
	}
	else
	{
		music.stop();
	}
}
//--------- End of function GameFile::load_process -------//


//------- Begin of function GameFile::write_game_header -------//
//
// Write saved game header info to the saved game file.
//
// Return : <int> 1 - file written successfully
//                0 - not successful
//
int GameFile::write_game_header(File* filePtr)
{
	class_size = sizeof(GameFile);

	Nation* playerNation = ~nation_array;

	strncpy( player_name, playerNation->king_name(), NationArray::HUMAN_NAME_LEN );
	player_name[NationArray::HUMAN_NAME_LEN] = NULL;

	race_id 		 = playerNation->race_id;
	nation_color = playerNation->nation_color;
	terrain_set  = config.terrain_set;

	game_date = info.game_date;

	//----- set the file date ------//

	CoFileTimeNow(&file_date);

	//------- write GameFile to the saved game file -------//

   return filePtr->file_write( this, sizeof(GameFile) );     // write the whole object to the saved game file
}
//--------- End of function GameFile::write_game_header -------//


//--------- Begin of function GameFile::validate_header -------//
int GameFile::validate_header()
{
	return class_size == sizeof(GameFile) && terrain_set > 0;
}
//--------- End of function GameFile::validate_header -------//
