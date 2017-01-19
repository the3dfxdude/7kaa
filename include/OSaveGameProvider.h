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
//Description : Provides the layer between the game (SaveGameArray) and the save backends.
//              Enumerates the savegames and provides actions for loading and saving.

#ifndef __OSAVEGAMEPROVIDER_H
#define __OSAVEGAMEPROVIDER_H

#include <functional>

class String;
struct SaveGameInfo;

// Note on nomenclature:
// - file path is the full path to the file.
// - file name is the last part of the file path.
// - Saved games are identified by their file names.
// - Scenario's are identified by their file path.


// Provides an abstraction layer between the UI for savegame handling, e.g. SaveGameArray, file handling, and the data reader/writer, GameFile.
// Handles identification of save game file names to save game file paths.
class SaveGameProvider
{
public:
	//  Enumerates all the savegames, calling callback for each savegame.
	static void enumerate_savegames(const char* filenameWildcard, const std::function<void (const SaveGameInfo* saveGameInfo)>& callback);

	// Deletes the savegame whose file part of filename is saveGameName.
	static void delete_savegame(const char* saveGameName);

	// Save the current game under the file specified by newFileName.
	static bool save_game(const char* newFileName, String& /*out*/ errorMessage);
	// Save the current game under the file specified by newFileName. Sets saveGameInfo to the new savegame information on success.
	static bool save_game(const char* newFileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage);

	// Loads the game given by fileName as the current game. Sets saveGameInfo to the new savegame information on success. Returns 1, 0, or -1 for resp. success, recoverable failure, or failure.
	static int load_game(const char* fileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage);

	// Loads the scenario whose full path is given by filePath. Returns 1, 0, or -1 for resp. success, recoverable failure, or failure.
	static int load_scenario(const char* filePath, String& /*out*/ errorMessage);

private:
	static int load_game_from_file(const char* filePath, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage);
};

#endif // !__OSAVEGAMEPROVIDER_H
