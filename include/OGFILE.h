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

//Filename    : OGFILE.H
//Description : Oject Game file, save and restore game

#ifndef __OGFILE_H
#define __OGFILE_H

class File;
class String;
struct SaveGameInfo;


//-------- Define static class GameFile -----------//

class GameFile
{
public:
   // Saves the current game under the given fileName in the given directory. Updates the saveGameInfo with the new savegame information. Returns true on success.
   static bool save_game(const char* directory, const char* fileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage);
   // Loads the saved game given by directory and fileName. Updates saveGameInfo in with the new savegame information. Returns 1, 0, or -1 for success, recoverable failure, failure.
   static int load_game(const char* filePath, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage);

   // Reads the given file and fills the save game info from the header. Returns true if successful.
   static bool read_header(const char* directory, const char* fileName, SaveGameInfo* /*out*/ saveGameInfo, String& /*out*/ errorMessage);

public:
   struct SaveGameHeader;

private:
   static bool validate_header(const SaveGameHeader* saveGame);

   static void  save_process();
   static void  load_process();
   static int   write_game_header(SaveGameInfo* /*in/out*/ saveGame, File* filePtr);

   static int   write_file(File*);
   static int   write_file_1(File*);
   static int   write_file_2(File*);
   static int   write_file_3(File*);

   static int   read_file(File*);
   static int   read_file_1(File*);
   static int   read_file_2(File*);
   static int   read_file_3(File*);

   static void  write_book_mark(File* filePtr, short bookMark);
   static int   read_book_mark(File* filePtr, short bookMark);

public:
	static bool read_file_same_version;				// true if major version of the game being loaded is same as that of the program

   // Static class has no constructors
private:
   GameFile() = delete;
   GameFile(const GameFile&) = delete;
};

#endif
