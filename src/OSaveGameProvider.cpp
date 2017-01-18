#include <OSaveGameProvider.h>
#include <OSaveGameInfo.h>
#include <OMISC.h>
#include <OFILE.h>
#include <ODIR.h>
#include <OSYS.h>
#include <OGFILE.h>
#include <OPOWER.h> // TODO: There might be an even better (higher-level / UI) place to do this (power.win_opened)
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
		SaveGameInfo saveGameInfo;
		String errorMessage;
		if( GameFile::read_header(directory, saveGameDirectory[i]->name, &saveGameInfo, /*out*/ errorMessage) )
		{
			saveGameInfo.file_date = saveGameDirectory[i]->time;
			callback(&saveGameInfo);
		}
		else
		{
			ERR("Failed to enumerate savegame '%s': %s\n", saveGameDirectory[i]->name, (const char*)errorMessage);
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