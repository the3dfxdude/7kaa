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

#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif

#ifndef __ONATION_H
#include <ONATION.h>
#endif


//------------ Define constant for game version == 1xx -----------------//
#define	VERSION_1_MAX_RACE						7
#define	VERSION_1_MAX_UNIT_TYPE					37
#define	VERSION_1_RACERES_NAME_COUNT			1582
#define	VERSION_1_UNITRES_UNIT_INFO_COUNT	37
#define	VERSION_1_TOWNRES_TOWN_NAME_COUNT	367
#define	VERSION_1_GODRES_GOD_COUNT				7
#define	VERSION_1_TECH_COUNT						7


//-------- Define struct HallFame ----------//

enum { HALL_FAME_NUM = 6 };     // No. of Hall of Fame entries

#pragma pack(1)
struct HallFame         // Hall of Fame
{
   char  player_name[NationArray::HUMAN_NAME_LEN+1];
   char  race_id;
   short start_year;
   short end_year;
   int   score;
   int   population;
   short difficulty_rating;

public:
   void  disp_info(int,int,int);
   void  record_data(int);
};
#pragma pack()

//-------- Define class GameFile -----------//

#pragma pack(1)
class GameFile
{
public:
   uint32_t class_size;    // for version compare
   char     file_name[MAX_PATH+1];

   char     player_name[NationArray::HUMAN_NAME_LEN+1];

   char     race_id;
   char     nation_color;

   int      game_date;      // the game date of the saved game
   FILETIME file_date;              // saving game date
   short    terrain_set;

   static   File* file_ptr;
	static   char  last_read_success_flag;

public:
   int   save_game(const char* =NULL);
   int   load_game(const char*, char*);

   void  set_file_name();
   void  disp_info(int x, int y);
   int   validate_header();

private:
   void  save_process();
   void  load_process();
   int   write_game_header(File* filePtr);

   int   write_file(File*);
   int   write_file_1(File*);
   int   write_file_2(File*);
   int   write_file_3(File*);

   int   read_file(File*);
   int   read_file_1(File*);
   int   read_file_2(File*);
   int   read_file_3(File*);

   void  write_book_mark(short bookMark);
   int   read_book_mark(short bookMark);
};
#pragma pack()

//------- Define class GameFileArray --------//

class GameFileArray : public DynArray
{
public:
   char     demo_format;       // whether write the game in shareware format or not (only selectable in design mode)
   char     has_read_hall_of_fame;
   char     last_file_name[MAX_PATH+1];

	short		load_file_game_version;	// game version of load file
	char		same_version;				// true if major version of the load game is same as that of the program

	HallFame hall_fame_array[HALL_FAME_NUM];

public:
   GameFileArray();

   void init(const char *extStr);
   void deinit();

   int  menu(int, int *recno=NULL);

   int  save_game()    { return menu(1); }
   int  load_game()    { return menu(2); }

   void save_new_game(const char* =NULL); // save a new game immediately without prompting menu

   int  read_hall_of_fame();
   int  write_hall_of_fame();    // it may be called by group_res.gen_group() in writting default name

   void disp_hall_of_fame();
   int  add_hall_of_fame(int);

   GameFile* operator[](int recNo);

private:
   void disp_browse();
   void load_all_game_header(const char *extStr);
   int  process_action(int=0);
   void del_game();
};

extern GameFileArray game_file_array;
extern GameFile      game_file;        //**BUGHERE

//-----------------------------------------

#endif
