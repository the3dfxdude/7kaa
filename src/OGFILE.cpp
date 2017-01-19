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

#include <OGFILE.h>
#include <OFILE.h>
#include <OSTR.h>
#include <OTALKRES.h>
#include <ONATION.h>
#include <OWORLD.h>
#include <OGAME.h>
#include <OTownNetwork.h>
#include <OINFO.h>
#include <OSYS.h>
#include <OAUDIO.h>
#include <OMUSIC.h>
#include <OSaveGameInfo.h>
#include <dbglog.h>
#include "gettext.h"

DBGLOG_DEFAULT_CHANNEL(GameFile);

#pragma pack(1)
struct GameFile::SaveGameHeader
{
	uint32_t class_size;    // for version compare
	SaveGameInfo info;
};
#pragma pack()

typedef GameFile::SaveGameHeader SaveGameHeader;
enum {CLASS_SIZE = 302};
static_assert(sizeof(SaveGameHeader) == CLASS_SIZE, "Savegame header size mismatch"); // (no packing)


//-------- Begin of function GameFile::save_game --------//
//
// Saves the current game under the given filePath.
// On error, returns false, and returns the error message in errorMessage.
//
bool GameFile::save_game(const char* filePath, const SaveGameInfo& saveGameInfo, String& /*out*/ errorMessage)
{
	File file;
	bool fileOpened = false;

	int rc = file.file_create(filePath, 0, 1); // 0=tell File don't handle error itself
												// 1=allow the writing size and the read size to be different
	if( !rc )
		errorMessage = _("Error creating saved game file.");
	else
		fileOpened = true;

	if( rc )
	{
		save_process();      // process game data before saving the game

		rc = write_game_header(saveGameInfo, &file);    // write saved game header information

		if( !rc )
			errorMessage = _("Error creating saved game header.");

		if( rc )
		{
			rc = write_file(&file);

			if( !rc )
				errorMessage = _("Error writing saved game data.");
		}
	}

	file.file_close();

	//------- when saving error ---------//

	if( !rc )
	{
		if (fileOpened) remove( filePath );         // delete the file as it is not complete

		errorMessage += _(" The game is not saved.");      // Also explicitly inform the user that the game has not been saved.
	}

	return rc != 0;
}
//--------- End of function GameFile::save_game --------//


//-------- Begin of function GameFile::load_game --------//
//
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int GameFile::load_game(const char* filePath, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	File file;
	int  rc=1;

	if(rc && !file.file_open(filePath, 0, 1)) // 0=tell File don't handle error itself
	{
		rc = 0;
		errorMessage = _("Cannot open save game file");
	}

	//-------- read in the GameFile class --------//

	SaveGameHeader saveGameHeader;
	if( rc )
	{
		if( !file.file_read(&saveGameHeader, CLASS_SIZE) )	// read the whole object from the saved game file
		{
			rc = 0;
			errorMessage = _("Cannot read file header");
		}
		else if( !validate_header(&saveGameHeader) )
		{
			rc = 0;
			errorMessage = _("Save game incompatible");
		}
	}

	//--------------------------------------------//
																  // 1=allow the writing size and the read size to be different
	if( rc )
	{
		config.terrain_set = saveGameHeader.info.terrain_set;

		game.deinit(1);		// deinit last game first, 1-it is called during loading of a game
		game.init(1);			// init game

		//-------- read in saved game ----------//

		switch( read_file(&file) )
		{
		case 1:
			rc = 1;
			break;
		case -1:
			rc = 0;		// consider cancel load game
			errorMessage = _("Incompatible save game");
			break;
		case 0:
		default:
			rc = -1;
			errorMessage = _("Load game error");
		}

		if( rc > 0 )
		{
			load_process();           // process game data after loading the game
			
			//------- create the town network --------//
			town_network_array.recreate_after_load();
		}
	}

	file.file_close();

	//---------------------------------------//

	if (rc > 0)
	{
		*saveGameInfo = saveGameHeader.info;
	}

	return rc;
}
//--------- End of function GameFile::load_game --------//


//-------- Begin of function GameFile::read_header --------//
//
// Reads the given file and fills the save game info from the header. Returns true if successful.
//
bool GameFile::read_header(const char* filePath, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	bool success;
	File file;
	SaveGameHeader saveGameHeader;
	if( file.file_open(filePath, 0, 1)      // last 1=allow varying read & write size
		&& file.file_read(&saveGameHeader, sizeof(SaveGameHeader)) )
	{
		if( !validate_header(&saveGameHeader) )
		{
			errorMessage = _("Invalid header");
			success = false;
		}
		else
		{
			success = true;
		}
	}
	else
	{
		errorMessage = _("Could not open file");
		success = false;
	}
	file.file_close();

	if (success)
	{
		*saveGameInfo = saveGameHeader.info;
	}

	return success;
}
//--------- End of function GameFile::read_header --------//


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
int GameFile::write_game_header(const SaveGameInfo& saveGameInfo, File* filePtr)
{
	SaveGameHeader saveGameHeader;
	saveGameHeader.class_size = CLASS_SIZE;
	saveGameHeader.info = saveGameInfo;
	return filePtr->file_write( &saveGameHeader, sizeof(SaveGameHeader) );     // write the whole object to the saved game file
}
//--------- End of function GameFile::write_game_header -------//


//--------- Begin of function GameFile::validate_header -------//
bool GameFile::validate_header(const SaveGameHeader* saveGameHeader)
{
	return saveGameHeader->class_size == CLASS_SIZE && saveGameHeader->info.terrain_set > 0;
}
//--------- End of function GameFile::validate_header -------//
