/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2016 Richard Dijk <microvirus.multiplying@gmail.com>
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

//Filename    : OSaveGameProvider.h
//Description : Provides the layer between the game (GameFileArray) and the save backends.
//              Enumerates the savegames and provides actions for loading and saving.

#ifndef __OSAVEGAMEPROVIDER_H
#define __OSAVEGAMEPROVIDER_H

#include <storage_constants.h>
#include <win32_compat.h>
#include <functional>


// The basic information ('header') of a savegame.
#pragma pack(1)
struct SaveGameInfo
{
	char     file_name[MAX_PATH+1];
	char     player_name[HUMAN_NAME_LEN+1];

	char     race_id;
	char     nation_color;

	int      game_date;      // the game date of the saved game
	FILETIME file_date;      // saving game date
	short    terrain_set;
};
#pragma pack()

// Provides an abstraction layer between the UI for savegame handling, GameFileArray, file handling, and the data reader/writer, GameFile.
class SaveGameProvider
{
public:
	//  Enumerates all the savegames, calling callback for each savegame.
	static void enumerate_savegames(const char* filenameWildcard, const std::function<void (const SaveGameInfo* saveGameInfo)>& callback);
};

#endif // !__OSAVEGAMEPROVIDER_H
