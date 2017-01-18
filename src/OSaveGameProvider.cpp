#include <OSaveGameProvider.h>
#include <OSaveGameInfo.h>
#include <OMISC.h>
#include <OFILE.h>
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
// Save the current game under the file specified by saveGameInfo. Updates saveGameInfo to the new savegame information on success.
//
bool SaveGameProvider::save_game(SaveGameInfo* /*in/out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	return save_game(saveGameInfo->file_name, /*out*/ saveGameInfo, /*out*/ errorMessage);
}
//-------- End of function SaveGameProvider::save_game(1) --------//


//-------- Begin of function SaveGameProvider::save_game(2) --------//
//
// Save the current game under the name given by newFileName.
//
bool SaveGameProvider::save_game(const char* newFileName, String& /*out*/ errorMessage)
{
	SaveGameInfo newSaveGameInfo;
	return save_game(newFileName, /*out*/ &newSaveGameInfo, /*out*/ errorMessage);
}
//-------- End of function SaveGameProvider::save_game(2) --------//


//-------- Begin of function SaveGameProvider::save_game(3) --------//
//
// Save the current game under the file specified by newFileName. Updates saveGameInfo to the new savegame information on success.
//
bool SaveGameProvider::save_game(const char* newFileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	power.win_opened=1;				// to disable power.mouse_handler()

	bool success;
	SaveGameInfo newSaveGameInfo;
	if (GameFile::save_game(sys.dir_config, newFileName, &newSaveGameInfo, /*out*/ errorMessage)) {
		*saveGameInfo = newSaveGameInfo;
		success = true;
	}
	else {
		success = false;
	}

	power.win_opened=0;

	return success;
}
//-------- End of function SaveGameProvider::save_game(3) --------//


//-------- Begin of function SaveGameProvider::load_game --------//
//
// Loads the game given by saveGameInfo as he current game. Updates saveGameInfo on success.
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int SaveGameProvider::load_game(SaveGameInfo* /*in/out*/ saveGameInfo, String& /*out*/ errorMessage)
{
	return load_game(saveGameInfo->file_name, /*out*/ saveGameInfo, /*out*/ errorMessage);
}

//-------- Begin of function SaveGameProvider::load_game --------//
//
// Loads the game given by fileName as the current game. Updates saveGameInfo to the new savegame information on success.
// return : <int> 1 - loaded successfully.
//                0 - not loaded.
//               -1 - error and partially loaded
//
int SaveGameProvider::load_game(const char* fileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage)
{
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