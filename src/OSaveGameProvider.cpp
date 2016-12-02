#include <OSaveGameProvider.h>
#include <OMISC.h>
#include <OFILE.h>
#include <ODIR.h>
#include <OSYS.h>
#include <OGFILE.h>
#include <dbglog.h>

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
