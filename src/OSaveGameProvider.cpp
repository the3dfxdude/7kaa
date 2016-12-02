#include <OSaveGameProvider.h>
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
	char full_path[MAX_PATH+1];
	if (!misc.path_cat(full_path, sys.dir_config, filenameWildcard, MAX_PATH))
	{
		ERR("Path to the config directory too long.\n");
		return;
	}

	Directory saveGameDirectory;
	saveGameDirectory.read(full_path, 0);  // 0-Don't sort file names

	//-------- read in the headers of all savegames -------//

	for( int i=1 ; i<=saveGameDirectory.size() ; i++ )
	{
		if (!misc.path_cat(full_path, sys.dir_config, saveGameDirectory[i]->name, MAX_PATH))
		{
			ERR("Path to the saved game too long.\n");
			continue;
		}

		File file;
		SaveGameHeader saveGameHeader;
		if( file.file_open(full_path, 1, 1)      // last 1=allow varying read & write size
			&& file.file_read(&saveGameHeader, sizeof(SaveGameHeader))
			&& GameFile::validate_header(&saveGameHeader) )
		{
			strcpy( saveGameHeader.info.file_name, saveGameDirectory[i]->name );  // in case that the name may be different
			saveGameHeader.info.file_date = saveGameDirectory[i]->time;
			callback(&saveGameHeader.info);
		}
		file.file_close();
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
	return save_game(saveGameInfo, saveGameInfo->file_name, /*out*/ errorMessage);
}
//-------- End of function SaveGameProvider::save_game(1) --------//


//-------- Begin of function SaveGameProvider::save_game(2) --------//
//
// Save the current game under the name given by newFileName. Updates saveGameInfo to the new savegame information on success.
//
bool SaveGameProvider::save_game(SaveGameInfo* /*in/out*/ saveGameInfo, const char* newFileName, String& /*out*/ errorMessage)
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
//-------- End of function SaveGameProvider::save_game(2) --------//
