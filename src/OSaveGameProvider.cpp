/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2016-2017 Richard Dijk <microvirus.multiplying@gmail.com>
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

//Filename    : OSaveGameProvider.cpp
//Description : Provides the interface between the UI and the save game handling logic


#include <OSaveGameProvider.h>
#include <OSaveGameInfo.h>
#include <OMISC.h>
#include <ODIR.h>
#include <OSYS.h>
#include <OGFILE.h>
#include <OPOWER.h> // TODO: There might be an even better (higher-level / UI) place to do this (power.win_opened)
#include <OMOUSECR.h>
#include <dbglog.h>

#include "gettext.h"

#ifdef NO_WINDOWS
#include <unistd.h>
#else
#include <io.h>
#endif

DBGLOG_DEFAULT_CHANNEL(SaveGameProvider);


//-------- Begin of function SaveGameProvider::enumerate_savegames --------//
//
// Enumerates all the savegames that match the wildcard pattern, calling callback for each savegame.
//
void SaveGameProvider::enumerate_savegames(const char* filenameWildcard, const std::function<void(const SaveGameInfo* saveGameInfo)>& callback)
{
	const char* const directory = sys.dir_config;

	char full_path[MAX_PATH+1];
	if (!misc.path_cat(full_path, directory, filenameWildcard, MAX_PATH))
	{
		ERR("Path to the config directory too long.\n");
		return;
	}

	Directory saveGameDirectory;
	saveGameDirectory.read(full_path, 0);  // 0-Don't sort file names

	//-------- read in the headers of all savegames -------//

	for( int i=1 ; i<=saveGameDirectory.size() ; i++ )
	{
		const char* const saveGameName = saveGameDirectory[i]->name;
		String errorMessage;
		char save_game_path[MAX_PATH+1];
		if (!misc.path_cat(save_game_path, directory, saveGameName, MAX_PATH))
		{
			ERR("Path to saved game '%s' too long\n", saveGameName);
			continue;
		}

		SaveGameInfo saveGameInfo;
		if( GameFile::read_header(save_game_path, &saveGameInfo, /*out*/ errorMessage) )
		{
			strncpy(saveGameInfo.file_name, saveGameName, MAX_PATH); // in case file names are different
			saveGameInfo.file_date = saveGameDirectory[i]->time;
			callback(&saveGameInfo);
		}
		else
		{
			ERR("Failed to enumerate savegame '%s': %s\n", saveGameName, (const char*)errorMessage);
		}
	}
}
//-------- End of function SaveGameProvider::enumerate_savegames --------//


//-------- Begin of function SaveGameProvider::delete_savegame --------//
//
// Deletes the savegame whose file part of filename is saveGameName.
//
void SaveGameProvider::delete_savegame(const char* saveGameName) {
	char full_path[MAX_PATH+1];
	if (!misc.path_cat(full_path, sys.dir_config, saveGameName, MAX_PATH))
	{
		ERR("Path to the saved game too long.\n");
		return;
	}

	unlink(full_path);
}
//-------- End of function SaveGameProvider::delete_savegame --------//


//-------- Begin of function SaveGameProvider::save_game(1) --------//
//
// Save the current game under the file specified by newFileName.
//
bool SaveGameProvider::save_game(const char* newFileName, String& /*out*/ errorMessage)
{
	SaveGameInfo newSaveGameInfo;
	return save_game(newFileName, /*out*/ &newSaveGameInfo, /*out*/ errorMessage);
}
//-------- End of function SaveGameProvider::save_game(1) --------//


//-------- Begin of function SaveGameProvider::save_game(2) --------//
//
// Save the current game under the file specified by newFileName. Sets saveGameInfo to the new savegame information on success.
//
bool SaveGameProvider::save_game(const char* newFileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	// Note: we cannot assume that newFileName and saveGameInfo->file_name are different addresses, so we must take care when copying between the two.

	bool success = true;

	char full_path[MAX_PATH+1];
	if (!misc.path_cat(full_path, sys.dir_config, newFileName, MAX_PATH))
	{
		success = false;
		errorMessage = _("Path too long to the saved game.");
	}

	power.win_opened=1;				// to disable power.mouse_handler()

	SaveGameInfo newSaveGameInfo = SaveGameInfoFromCurrentGame(newFileName);
	success = success && GameFile::save_game(full_path, newSaveGameInfo, /*out*/ errorMessage);

	power.win_opened=0;

	if (success)
	{
		*saveGameInfo = newSaveGameInfo;
	}
	return success;
}
//-------- End of function SaveGameProvider::save_game(2) --------//


//-------- Begin of function SaveGameProvider::load_game --------//
//
// Loads the game given by fileName as the current game. Sets saveGameInfo to the new savegame information on success.
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int SaveGameProvider::load_game(const char* fileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	// Note: we cannot assume that fileName and saveGameInfo->file_name are different addresses, so we must take care when copying between the two.

	int rc = 1;
	char full_path[MAX_PATH+1];
	if (!misc.path_cat(full_path, sys.dir_config, fileName, MAX_PATH))
	{
		rc = 0;
		errorMessage = _("Path too long to the saved game");
	}

	if (rc > 0)
	{
		rc = load_game_from_file(full_path, /*out*/ saveGameInfo, /*out*/ errorMessage);
	}

	if (rc > 0)
	{
		// Fixup file name if stored file_name is different than actual file name
		if (saveGameInfo->file_name != fileName /*(comparison as pointers)*/) {
			strncpy(saveGameInfo->file_name, fileName, MAX_PATH);
			saveGameInfo->file_name[MAX_PATH] = '\0';
		}
	}

	return rc;
}
//-------- End of function SaveGameProvider::load_game --------//


//-------- Begin of function SaveGameProvider::load_scenario --------//
//
// Loads the scenario whose full path is given by filePath.
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int SaveGameProvider::load_scenario(const char* filePath, String& /*out*/ errorMessage)
{
	SaveGameInfo saveGameInfo;
	return load_game_from_file(filePath, /*out*/ &saveGameInfo, /*out*/ errorMessage);
}
//-------- End of function SaveGameProvider::load_scenario --------//


//-------- Begin of function SaveGameProvider::load_game_from_file --------//
//
// Loads the game given by the full filePath as the current game. Updates saveGameInfo to the new savegame information on success.
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int SaveGameProvider::load_game_from_file(const char* filePath, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	power.win_opened=1;				// to disable power.mouse_handler()
	const int oldCursor = mouse_cursor.get_icon();
	mouse_cursor.set_icon( CURSOR_WAITING );
	const int powerEnableFlag = power.enable_flag;

	int rc = GameFile::load_game(filePath, /*out*/ saveGameInfo, /*out*/ errorMessage);

	mouse_cursor.set_frame(0);		// to fix a frame bug with loading game

	power.enable_flag = powerEnableFlag;
	mouse_cursor.restore_icon( oldCursor );
	power.win_opened=0;

	return rc;
}
//-------- End of function SaveGameProvider::load_game_from_file --------//