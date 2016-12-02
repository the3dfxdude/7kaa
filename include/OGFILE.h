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

#ifndef __OFILE_H
#include <OFILE.h>
#endif

#include <OSaveGameInfo.h>


//------------ Define constant for game version == 1xx -----------------//
#define	VERSION_1_MAX_RACE						7
#define	VERSION_1_MAX_UNIT_TYPE					37
#define	VERSION_1_RACERES_NAME_COUNT			1582
#define	VERSION_1_UNITRES_UNIT_INFO_COUNT	37
#define	VERSION_1_TOWNRES_TOWN_NAME_COUNT	367
#define	VERSION_1_GODRES_GOD_COUNT				7
#define	VERSION_1_TECH_COUNT						7


#pragma pack(1)
struct SaveGameHeader
{
	uint32_t class_size;    // for version compare
	SaveGameInfo info;
};
#pragma pack()


//-------- Define static class GameFile -----------//

class GameFile
{
public:
   static   File* file_ptr;
   static   char  last_read_success_flag;

public:
   static int save_game(SaveGameInfo* /*in/out*/ saveGame, const char* =NULL);
   static int load_game(SaveGameInfo* /*in/out*/ saveGame, const char*, char*);

   static bool validate_header(const SaveGameHeader* saveGame);

private:
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

   static void  write_book_mark(short bookMark);
   static int   read_book_mark(short bookMark);

   // Static class has no constructors
private:
   GameFile() = delete;
   GameFile(const GameFile&) = delete;
};

#endif
