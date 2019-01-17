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

//------- Begin of function PlayerStats::load_scenario_file ------//
//
bool PlayerStats::load_scenario_file() {
	FilePath full_path(sys.dir_config);
	int  rc;
	File file;

	full_path += scn_dat_file;
	if (full_path.error_flag)
		return 0;

	if (!misc.is_file_exist(full_path))
		return 0;

	rc = file.file_open(full_path, 0, 1);   // 0=don't handle error itself
											// 1=allow the writing size and the read size to be different
	if (!rc)
		return false;
	// 1=allow the writing size and the read size to be different
	//--------- Read Hall of Fame ----------//

	if (rc) {
		scn_stat_arr_len = file.file_get_long();
	}

	if (rc && scn_stat_arr_len) {
		scn_stat_arr = (ScenStat *)mem_add_clear(sizeof(ScenStat)*(scn_stat_arr_len));
		rc = file.file_read(scn_stat_arr, sizeof(ScenStat) * scn_stat_arr_len);
	}

	file.file_close();
	return rc == 1 ? true : false;
}
//------- End of function PlayerStats::load_scenario_file ------//

//------- Begin of function PlayerStats::write_scenario_file ------//
//
// Will re-write the entire file with values from scn_stat_arr
//
bool PlayerStats::write_scenario_file() {

	FilePath full_path(sys.dir_config);
	full_path += scn_dat_file;
	if (full_path.error_flag)
		return 0;

	File file;
	int rc = file.file_create(full_path, 0, 1);  // 0=don't handle error itself

	if (!rc) { return false; }


	if (rc) {
		rc = file.file_put_long(scn_stat_arr_len);
	}

	if (rc) {
		rc = file.file_write(scn_stat_arr, sizeof(ScenStat) * scn_stat_arr_len);
	}

	file.file_close();
	return rc == 1 ? true : false;
}
//------- End of function PlayerStats::write_scenario_file ------//

//------- Begin of function PlayerStats::get_scenario_play_status ------//
//
PlayStatus PlayerStats::get_scenario_play_status(char const * name) {
	// Check if we already loaded our file. If not, do it.
	if (!scn_stat_arr) {
		if (!load_scenario_file()) {
			// New installs won't have the file, so we assume unplayed. The
			// file will get created the first time they play a scenario.
			return PlayStatus::UNPLAYED;
		}
	}

	// Find a matching name and return its status
	for (int i = 0; i < scn_stat_arr_len; i++) {
		if (!strncmp(scn_stat_arr[i].internal_name, name, detail::MAX_FILE_PATH)) {
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
bool PlayerStats::set_scenario_play_status(char const * name, PlayStatus status) {
	for (int i = 0; i < scn_stat_arr_len; i++) {
		if (!strncmp(scn_stat_arr[i].internal_name, name, detail::MAX_FILE_PATH)) {
			if (scn_stat_arr[i].status != status) {
				// Update the entry and re-write the file
				scn_stat_arr[i].status = status;
				return write_scenario_file();
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
	strncpy(scn_stat_arr[idx].internal_name, name, detail::MAX_FILE_PATH);
	scn_stat_arr[idx].status = status;
	return write_scenario_file();
}
//------- End of function PlayerStats::set_scenario_play_status ------//
