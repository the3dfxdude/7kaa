/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2019 Steven Lavoie
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

//Filename    : PlayerStats.h
//Description : Handles file IO for player statistics information

#ifndef __PLAYERSTATS_H
#define __PLAYERSTATS_H

#include <OGAME.h>
#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif


namespace nsPlayerStats {

enum PlayStatus : int { UNPLAYED = 0, PLAYED = 1, COMPLETED = 2 };

namespace detail {

char const * const scn_dat_file = "PLAYSTAT.DAT";
enum { MAX_FILE_PATH = 260 }; //HACK: This is repeated all over. Should be global constant.

//
// This structure is written as-is to PLAYSTAT.DAT
//
#pragma pack(1)
typedef struct {
	//
	// This size is dictated by SAV format and is a legacy requirement
	//
	char internal_name[MAX_FILE_PATH + 1];
	PlayStatus status;
	//
	// Room for expansion later. Just subtract
	// the size of your new value from this
	//
	char reserved_for_later[120];
} ScenStat;

static_assert(sizeof(ScenStat::internal_name) == MAX_FILE_PATH + 1, "Changing ScenStat is a breaking change for PLAYSTAT.DAT");
static_assert(sizeof(ScenStat) == 385, "Changing ScenStat is a breaking change for PLAYSTAT.DAT");

}

class PlayerStats
{
private:
	detail::ScenStat * scn_stat_arr;
	size_t scn_stat_arr_len;

	bool write_scenario_file();

public:
	PlayStatus get_scenario_play_status(char const * name);
	bool set_scenario_play_status(char const * name, PlayStatus status);
	// If force_reload==true, the stats will be reloaded from the file or,
	// if the file is deleted, the UI will be updated to reflect that
	bool load_scenario_file(bool force_reload = false);

	PlayerStats();
	~PlayerStats();
};
} // nsPlayerStats

extern nsPlayerStats::PlayerStats playerStats; // in AM.cpp
#endif
