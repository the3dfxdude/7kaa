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

//Filename    : OSaveGameArray.h
//Description : Save/load game menu

#ifndef __OSAVEGAMEARRAY_H
#define __OSAVEGAMEARRAY_H

#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif

#include <ODIR.h>
#include <OSaveGameInfo.h>

struct SaveGame
{
	FileInfo file_info;
	SaveGameInfo header;
};

class SaveGameArray : private DynArray
{
public:
	SaveGameArray();

	void init(const char *extStr);
	void deinit();

	int  menu(int, int *recno=NULL);

	int  save_game()    { return menu(1); }
	int  load_game()    { return menu(2); }

	int save_new_game(const char* =NULL); // save a new game immediately without prompting menu

	SaveGame* operator[](int recNo);

private:
	void set_file_name(char* /*out*/ fileName, int size);

	void disp_browse();
	static void disp_entry_info(const SaveGame* entry, int x, int y);
	void load_all_game_header(const char *extStr);
	int  process_action(int=0);
	void del_game();

private:
	bool     has_fetched_last_file_name_from_hall_of_fame;
	char     last_file_name[MAX_PATH+1]; // (persisted via HallOfFame)
};

extern SaveGameArray save_game_array;

#endif
