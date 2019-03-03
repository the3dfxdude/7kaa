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

//Filename    : PlayerStats.cpp
//Description : Handles file IO for player statistics information

#include <PlayerStats.h>
#include <OSYS.h>

extern Sys sys;			// For the config directory
using namespace nsPlayerStats;
using namespace detail;


//------- Begin of function PlayerStats::PlayerStats ------//
//
PlayerStats::PlayerStats() :
	scn_stat_arr(nullptr),
	scn_stat_arr_len(0) { }
//------- End of function PlayerStats::PlayerStats ------//

//------- Begin of function PlayerStats::~PlayerStats ------//
//
PlayerStats::~PlayerStats() {
	if (scn_stat_arr) {
		mem_del(scn_stat_arr);
		scn_stat_arr = nullptr; // used as a flag later
	}
}
//------- End of function PlayerStats::~PlayerStats ------//

//------- Begin of function PlayerStats::load_player_stats ------//
//
bool PlayerStats::load_player_stats(bool force_reload) {
	FilePath full_path(sys.dir_config);
	int  rc;
	File file;

	full_path += scn_dat_file;
	if (full_path.error_flag)
		return false;

	if (!misc.is_file_exist(full_path))
		return false;

	rc = file.file_open(full_path, 0, 0);

	if (!rc) { return false; }

	//--------- Read Record Header ----------//

	RecordHeader hdr;
	rc = file.file_read(&hdr, sizeof(RecordHeader));
	if (!rc) { return false; }

	//--------- Read Record Data ----------//

	//
	// Since not every install will have records for the various
	// statistics, we can't assume they can be read in any order
	// or even that they're present at all.
	//

	switch (hdr.rec_type) {
	case RecordType::ScenarioPlayStatus:
		if (scn_stat_arr && !force_reload) {
			return true;
		} else if (scn_stat_arr && force_reload) {
			mem_del(scn_stat_arr);
			scn_stat_arr = nullptr;
			scn_stat_arr_len = 0;
		}
		scn_stat_arr = (ScenStat *)mem_add_clear(hdr.rec_size*hdr.rec_count);
		scn_stat_arr_len = hdr.rec_count;
		rc = file.file_read(scn_stat_arr, hdr.rec_size*hdr.rec_count);
	default:
		rc = 0;
	}

	file.file_close();
	return rc == 1 ? true : false;
}
//------- End of function PlayerStats::load_player_stats ------//

//------- Begin of function PlayerStats::write_player_stats ------//
//
// Will (re)write the entire file
//
bool PlayerStats::write_player_stats() {
	FilePath full_path(sys.dir_config);
	full_path += scn_dat_file;
	if (full_path.error_flag) { return 0; }

	File file;
	int rc = file.file_create(full_path, 0, 0);
	if (!rc) { return false; }

	//
	// Note, the order of the records doesn't matter since
	// we always read/write a RecordHeader first, which tells
	// us how many bytes follow.
	//

	//--------- Write Record Header ----------//
	RecordHeader hdr;
	hdr.rec_count = scn_stat_arr_len;
	hdr.rec_type = RecordType::ScenarioPlayStatus;
	hdr.rec_size = sizeof(ScenStat);
	rc = file.file_write(&hdr, sizeof(RecordHeader));
	if (!rc) { return false; }

	//--------- Write Record Data ----------//
	rc = file.file_write(scn_stat_arr, hdr.rec_size*hdr.rec_count);

	file.file_close();
	return rc == 1 ? true : false;
}
//------- End of function PlayerStats::write_player_stats ------//

//------- Begin of function PlayerStats::get_scenario_play_status ------//
//
PlayStatus PlayerStats::get_scenario_play_status(char const * game_name) {
	// Check if we already loaded our file. If not, do it.
	if (!scn_stat_arr) {
		if (!load_player_stats()) {
			// New installs won't have the file, so we assume unplayed. The
			// file will get created the first time they play a scenario.
			return PlayStatus::UNPLAYED;
		}
	}

	// Find a matching name and return its status
	for (int i = 0; i < scn_stat_arr_len; i++) {
		if (!strncmp(scn_stat_arr[i].game_name, game_name, detail::MAX_FILE_PATH)) {
			return scn_stat_arr[i].status;
		}
	}

	// If the file failed to load or we don't have a 
	// matching record, default to unplayed.
	return PlayStatus::UNPLAYED;
}
//------- End of function PlayerStats::get_scenario_play_status ------//

//------- Begin of function PlayerStats::set_scenario_play_status ------//
//
bool PlayerStats::set_scenario_play_status(char const * game_name, PlayStatus status) {
	// Check if we already loaded our file. If not, do it.
	if (!scn_stat_arr) {
		// If it fails, we'll create a new one. If it succeeds, we'll
		// write to it. No need to check the return value here.
		load_player_stats();
	}

	for (int i = 0; i < scn_stat_arr_len; i++) {
		if (!strncmp(scn_stat_arr[i].game_name, game_name, detail::MAX_FILE_PATH)) {
			if (scn_stat_arr[i].status != status) {
				// Update the entry and re-write the file
				scn_stat_arr[i].status = status;
				return write_player_stats();
			} else {
				// Don't bother writing to the file if no change is needed
				return true;
			}
		}
	}

	// If we get here, there was no entry so we need a new one
	int idx = scn_stat_arr_len;
	scn_stat_arr_len++;
	scn_stat_arr = (ScenStat *)mem_resize(scn_stat_arr, sizeof(ScenStat)*(scn_stat_arr_len));
	strncpy(scn_stat_arr[idx].game_name, game_name, detail::MAX_FILE_PATH);
	scn_stat_arr[idx].status = status;
	return write_player_stats();
}
//------- End of function PlayerStats::set_scenario_play_status ------//
