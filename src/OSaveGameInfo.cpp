/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
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

//Filename    : OSaveGameInfo.cpp
//Description : Helper methods for SaveGameInfo

#include <OSaveGameInfo.h>
#include <ONATION.h>
#include <OCONFIG.h>
#include <OINFO.h>

#ifndef NO_WINDOWS
#include <Windows.h>
#endif


SaveGameInfo SaveGameInfoFromCurrentGame(const char* newFileName)
{
	SaveGameInfo saveGameInfo;

	strncpy( saveGameInfo.file_name, newFileName, MAX_PATH);
	saveGameInfo.file_name[MAX_PATH] = '\0';

	Nation* playerNation = ~nation_array;
	strncpy( saveGameInfo.player_name, playerNation->king_name(), HUMAN_NAME_LEN );
	saveGameInfo.player_name[HUMAN_NAME_LEN] = '\0';

	saveGameInfo.race_id      = playerNation->race_id;
	saveGameInfo.nation_color = playerNation->nation_color;

	saveGameInfo.game_date    = info.game_date;
	saveGameInfo.file_date    = 0;
#ifndef NO_WINDOWS  // FIXME
	//----- set the file date ------//

	FILETIME sysTimeAsFileTime;
	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	SystemTimeToFileTime(&sysTime, &sysTimeAsFileTime);
	saveGameInfo.file_date = static_cast<std::uint64_t>(sysTimeAsFileTime.dwHighDateTime) << 32 | sysTimeAsFileTime.dwLowDateTime;
#endif

	saveGameInfo.terrain_set  = config.terrain_set;

	return saveGameInfo;
}
