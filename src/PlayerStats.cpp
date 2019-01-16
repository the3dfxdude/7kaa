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

using namespace player_stat_detail;
extern Sys sys;

PlayerStats::PlayerStats() {

}

PlayerStats::~PlayerStats() {
	if (scene_stats) {
		mem_del(scene_stats);
		scene_stats = nullptr; // used as a flag later
	}
}

FILE * PlayerStats::open_scenario_file(bool read) {
	String filePath(sys.dir_config);
	if (!misc.mkpath(sys.dir_config)) {
		filePath += ".";
	}

	filePath += PATH_DELIM;
	filePath += "scn_list.txt";
	char * method = read ? "r" : "w";
	return fopen(filePath, method);
}

bool PlayerStats::load_scenario_file() {
	FILE *f = open_scenario_file(true);
	if (!f) { return false; }

	String line;
	while (fgets(line, player_stat_detail::MAX_FILE_PATH, f)) {
		int idx = scene_stats_len;
		scene_stats_len++;
		scene_stats = (ScenarioStats *)mem_resize(scene_stats, sizeof(ScenarioStats)*(scene_stats_len));
		if (!scene_stats) { return false; }

		strncpy(scene_stats[idx].internal_name, line, player_stat_detail::MAX_FILE_PATH);
		int len = strnlen(scene_stats[idx].internal_name, player_stat_detail::MAX_FILE_PATH);
		scene_stats[idx].internal_name[len - 1] = '\0'; //trim newline
		fgets(line, 10, f);
		char token = line[7];
		int val = token - '0';
		scene_stats[idx].status = static_cast<PlayStatus>(val);
	}

	fclose(f);
	return true;
}

PlayStatus PlayerStats::get_scenario_play_status(char const * name) {
	// Check if we already loaded our file. If not, do it.
	if (!scene_stats) {
		if (!load_scenario_file()) {
			// Users just getting this new feature won't have the file,
			// so we assume unplayed. A file will get created the first
			// time they play a scenario.
			return PlayStatus::UNPLAYED;
		}
	}

	// Find a matching name and return its status
	for (int i = 0; i < scene_stats_len; i++) {
		if (!strncmp(scene_stats[i].internal_name, name, player_stat_detail::MAX_FILE_PATH)) {
			return scene_stats[i].status;
		}
	}

	// If the file failed to load or we don't have a 
	// matching record, default to unplayed.
	return PlayStatus::UNPLAYED;
}

bool PlayerStats::save_scenario_stat(char const * name, PlayStatus status) {
	
	bool found = false;
	for (int i = 0; i < scene_stats_len; i++) {
		if (!strncmp(scene_stats[i].internal_name, name, player_stat_detail::MAX_FILE_PATH)) {
			scene_stats[i].status = status;
			found = true;
		}
	}

	if (!found) {
		int idx = scene_stats_len;
		scene_stats_len++;
		scene_stats = (ScenarioStats *)mem_resize(scene_stats, sizeof(ScenarioStats)*(scene_stats_len));
		strncpy(scene_stats[idx].internal_name, name, player_stat_detail::MAX_FILE_PATH);
		scene_stats[idx].status = status;
	}

	//
	// We re-write the entire file. If we get more scenarios or
	// more data, this will need to just update an entry.
	//
	FILE *f = open_scenario_file(false);
	if (!f) { return false; }
	for (int i = 0; i < scene_stats_len; i++) {
		fprintf(f, "%s\n", scene_stats[i].internal_name);	// write scenario name
		fprintf(f, "Status:");								// write status token
		fprintf(f, "%d\n", scene_stats[i].status);			// write status value
		fclose(f);
	}
	
	return true;
}