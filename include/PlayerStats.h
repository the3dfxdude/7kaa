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

enum PlayStatus : uint32_t { UNPLAYED = 0, PLAYED = 1, COMPLETED = 2 };
enum RecordType : uint32_t { ScenarioPlayStatus = 0 };

namespace detail {

char const * const scn_dat_file = "PLAYSTAT.DAT";
enum { MAX_FILE_PATH = 260 }; //HACK: This is repeated all over. Should be global constant.

//
// These structures are written as-is to PLAYSTAT.DAT and cannot be changed
// without breaking the format. To prevent breaking changes, just add a new
// record type that includes whatever additional data.
//

//
// Dictates the reading/writing of the file. Every type of statistic
// must be preceded by a RecordHeader. This struct is written as-is
// to/from an array and file so ensure any changes are 4-byte aligned.
//
struct RecordHeader {
	RecordType rec_type;
	uint32_t rec_count;
	uint32_t rec_size;
};
static_assert(sizeof(RecordHeader) == 12, "Changing RecordHeader is a breaking change for PLAYSTAT.DAT");

//
// Tracks whether a scenario has been played/completed. This
// struct is written as-is to/from an array and file so ensure
// any changes are 4-byte aligned.
//
struct ScenStat {
	//
	// The SAV format actually has MAX_FILE_PATH + 1, which was stupid. Since
	// this field is not actively used and even legacy scenario files only contain
	// an 8.3 filename, we'll dispense with the extra byte. Windows includes /0 in
	// its MAX_PATH's 260 bytes anyway.
	//
	char internal_name[MAX_FILE_PATH];
	PlayStatus status;
};

static_assert(sizeof(ScenStat::internal_name) == MAX_FILE_PATH, "Changing ScenStat is a breaking change for PLAYSTAT.DAT");
static_assert(sizeof(ScenStat) == 264, "Changing ScenStat is a breaking change for PLAYSTAT.DAT");

}

class PlayerStats
{
private:
	detail::ScenStat * scn_stat_arr;
	size_t scn_stat_arr_len;

	bool write_player_stats();

public:
	PlayStatus get_scenario_play_status(char const * name);
	bool set_scenario_play_status(char const * name, PlayStatus status);
	// If force_reload==true, the stats will be reloaded from the file or,
	// if the file is deleted, the UI will be updated to reflect that
	bool load_player_stats(bool force_reload = false);

	PlayerStats();
	~PlayerStats();
};
} // nsPlayerStats

extern nsPlayerStats::PlayerStats playerStats; // in AM.cpp
#endif
