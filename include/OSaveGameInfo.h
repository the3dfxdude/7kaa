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

//Filename    : OSaveGameInfo.h
//Description : Defines the basic information of a savegame. Also part of the header (SaveGameHeader) written as binary to the V2 savegames.

#ifndef __OSAVEGAMEINFO_H
#define __OSAVEGAMEINFO_H

#include <storage_constants.h>

#include <cstdint>

#ifdef USE_WINDOWS
#include <windows.h>
#else
typedef struct _FILETIME {
	uint32_t dwLowDateTime;
	uint32_t dwHighDateTime;
} FILETIME, *PFILETIME;
#endif

// The basic information ('header') of a savegame.
#pragma pack(1)
struct SaveGameInfo
{
	char     file_name[MAX_PATH+1]; // unused
	char     player_name[HUMAN_NAME_LEN+1];

	char     race_id;
	char     nation_color;

	int           game_date;      // the game date of the saved game
	FILETIME      file_date;      // unused
	short         terrain_set;
};
#pragma pack()

SaveGameInfo SaveGameInfoFromCurrentGame(const char* newFileName);

#endif // ! __OSAVEGAMEINFO_H
