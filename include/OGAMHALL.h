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

#ifndef __OGAMHALL_H
#define __OGAMHALL_H

//Filename    : OGAMHALL.H
//Description : Hall of Fame

#include <storage_constants.h>


//-------- Define struct HallFameEntry ----------//

enum { HALL_FAME_NUM = 6 };     // No. of Hall of Fame entries

#pragma pack(1)
struct HallFameEntry         // Hall of Fame
{
	char  player_name[HUMAN_NAME_LEN+1];
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

class HallOfFame {
public:
	HallOfFame();

	void init();
	void deinit();

	const char* const get_last_savegame_file_name() const {return last_savegame_file_name;}
	void set_last_savegame_file_name(const char* fileName);

	int  add_hall_of_fame(int totalScore);
	void disp_hall_of_fame();

private:
	int  read_hall_of_fame();
	int  write_hall_of_fame();    // it may be called by group_res.gen_group() in writting default name

private:
	HallFameEntry hall_fame_array[HALL_FAME_NUM];
	// In a slight (historical) abuse of responsibility, HALLFAME.DAT contains the last savegame name. SaveGameArray maintains it during runtime, however.
	char     last_savegame_file_name[MAX_PATH+1];
};

extern HallOfFame hall_of_fame;

#endif // !__OGAMHALL_H
