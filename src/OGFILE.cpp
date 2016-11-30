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
#include <OTownNetwork.h>
#include <OINFO.h>
#include <OGFILE.h>
#include <OSYS.h>
#include <OAUDIO.h>
#include <OMUSIC.h>
#include <dbglog.h>
#include "gettext.h"

DBGLOG_DEFAULT_CHANNEL(GameFile);

// -------- define constant ----------//
#define MIN_FREE_SPACE 1000

enum {CLASS_SIZE = 302};
static_assert(sizeof(SaveGameHeader) == CLASS_SIZE, "Savegame header size mismatch"); // (no packing)


//-------- Begin of function GameFile::save_game --------//
//
// return : <int> 1 - saved successfully.
//                0 - not saved.
//
int GameFile::save_game(SaveGameInfo* /*in/out*/ saveGame, const char* fileName)
{
	char full_path[MAX_PATH+1];
	File   file;
	String errStr;
	bool fileOpened = false;

	power.win_opened=1;				// to disable power.mouse_handler()

	if( fileName )
		strcpy( saveGame->file_name, fileName );

	int rc = 1;

	if (!misc.path_cat(full_path, sys.dir_config, saveGame->file_name, MAX_PATH))
	{
		rc = 0;
		errStr = _("Path too long to the saved game.");
	}

	if( rc )
	{
		rc = file.file_create(full_path, 0, 1); // 0=tell File don't handle error itself
																   // 1=allow the writing size and the read size to be different
		if( !rc )
			errStr = _("Error creating saved game file.");
		else
			fileOpened = true;
	}

	if( rc )
	{
		save_process();      // process game data before saving the game

		rc = write_game_header(saveGame, &file);    // write saved game header information

		if( !rc )
			errStr = _("Error creating saved game header.");

		if( rc )
		{
			rc = write_file(&file);

			if( !rc )
				errStr = _("Error writing saved game data.");
		}
	}

	file.file_close();

	power.win_opened=0;

	//------- when saving error ---------//

	if( !rc )
	{
		if (fileOpened) remove( saveGame->file_name );         // delete the file as it is not complete

		errStr += _(" The game is not saved.");      // Also explicitly inform the user that the game has not been saved.

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
int GameFile::load_game(SaveGameInfo* /*in/out*/ saveGame, const char *base_path, char* fileName)
{
	char full_path[MAX_PATH+1];
	File file;
	int  rc=0;
	const char *errMsg = NULL;

	power.win_opened=1;				// to disable power.mouse_handler()

	int oldCursor = mouse_cursor.get_icon();
	mouse_cursor.set_icon( CURSOR_WAITING );

	int powerEnableFlag = power.enable_flag;

	if( fileName )
		strcpy( saveGame->file_name, fileName );

	rc = 1;

	if (!misc.path_cat(full_path, base_path, saveGame->file_name, MAX_PATH))
	{
		rc = 0;
		errMsg = _("Path too long to the saved game");
	}

	if(rc && !file.file_open(full_path, 0, 1)) // 0=tell File don't handle error itself
	{
		rc = 0;
		errMsg = _("Cannot open save game file");
	}

	//-------- read in the GameFile class --------//

	if( rc )
	{
		char gameFileName[MAX_PATH+1];

		strcpy( gameFileName, saveGame->file_name );     // save the file name actually read, in case that the file names are different

		MSG(__FILE__":%d: file_read(this, ...);\n", __LINE__);

		SaveGameHeader saveGameHeader;
		if( !file.file_read(&saveGameHeader, CLASS_SIZE) )	// read the whole object from the saved game file
		{
			rc = 0;
			errMsg = _("Cannot read file header");
		}
		if( rc )
		{
			*saveGame = saveGameHeader.info;
			if( !validate_header(&saveGameHeader) )
			{
				rc = 0;
				errMsg = _("Save game incompatible");
			}
			else
				strcpy( saveGame->file_name, gameFileName );
		}
	}

	//--------------------------------------------//
																  // 1=allow the writing size and the read size to be different
	if( rc )
	{
		config.terrain_set = saveGame->terrain_set;

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
			errMsg = _("Incompatible save game");
			break;
		case 0:
		default:
			rc = -1;
			errMsg = _("Load game error");
		}

		if( rc > 0 )
		{
			load_process();           // process game data after loading the game
			
			//------- create the town network --------//
			town_network_array.recreate_after_load();
			
		}
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


//-------- Begin of function GameFile::save_process -------//
//
// Make the game data ready for saving game
//
// Called before saving the game
//
void GameFile::save_process()
{
   //--------- set the total playing time --------//

	info.total_play_time += misc.get_time()-info.start_play_time;

   info.start_play_time  = misc.get_time();
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
	info.start_play_time = misc.get_time();       // the time player start playing the game
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
int GameFile::write_game_header(SaveGameInfo* /*in/out*/ saveGame, File* filePtr)
{
	Nation* playerNation = ~nation_array;

	strncpy( saveGame->player_name, playerNation->king_name(), HUMAN_NAME_LEN );
	saveGame->player_name[HUMAN_NAME_LEN] = '\0';

	saveGame->race_id      = playerNation->race_id;
	saveGame->nation_color = playerNation->nation_color;
	saveGame->terrain_set  = config.terrain_set;

	saveGame->game_date    = info.game_date;

#ifndef NO_WINDOWS  // FIXME
	//----- set the file date ------//

	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	SystemTimeToFileTime(&sysTime, &saveGame->file_date);
#endif

	//------- write GameFile to the saved game file -------//

	SaveGameHeader saveGameHeader;
	saveGameHeader.class_size = CLASS_SIZE;
	saveGameHeader.info = *saveGame;
	return filePtr->file_write( &saveGameHeader, sizeof(SaveGameHeader) );     // write the whole object to the saved game file
}
//--------- End of function GameFile::write_game_header -------//


//--------- Begin of function GameFile::validate_header -------//
bool GameFile::validate_header(const SaveGameHeader* saveGameHeader)
{
	return saveGameHeader->class_size == CLASS_SIZE && saveGameHeader->info.terrain_set > 0;
}
//--------- End of function GameFile::validate_header -------//
