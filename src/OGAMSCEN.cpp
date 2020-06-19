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

//Filename    : OGAMSCEN.CPP
//Description : Select Game Scenario

#include <OSTR.h>
#include <OSYS.h>
#include <ONEWS.h>
#include <ODATE.h>
#include <OGFILE.h>
#include <OSaveGameArray.h>
#include <OSaveGameProvider.h>
#include <OF_MONS.h>
#include <OMONSRES.h>
#include <OFILETXT.h>
#include <ODIR.h>
#include <OBOX.h>
#include <OBATTLE.h>
#include <OGAME.h>
#include <ONATIONA.h>
#include <PlayerStats.h>
#include <ConfigAdv.h>
#include <FileSystem.h>

//--------- declare static vars ----------//

static void init_scenario_var(ScenInfo* scenInfo);
static int sort_scenario_func(const void *arg1, const void *arg2);
using namespace nsPlayerStats;

//---------- Begin of function Game::select_run_scenario ----------//
//
// Select and play a scenario.
//
// return : <int> 1 - ok, a scenario is selected and run
//                0 - cancel
//
int Game::select_run_scenario()
{
	Directory gameDirList[MAX_SCENARIO_PATH];
	ScenInfo* scenInfoArray = NULL;
	int scenInfoSize = 0;
	int dirId;

	for( dirId = 0; dirId < MAX_SCENARIO_PATH; ++dirId )
	{
		if( DIR_SCENARIO_PATH(dirId)[0] )
		{
			Directory &gameDir = gameDirList[dirId];
			{
				String str;
				str  = DIR_SCENARIO_PATH(dirId);
				str += "*.SCN";
				gameDir.read( str, 1 );     // Read in all file names with the "SCN" extension
			}

			if( gameDir.size() > 0)
			{
				//---- append scenario file names in this directory into an array ----//

				scenInfoArray = (ScenInfo *)mem_resize( scenInfoArray, sizeof(ScenInfo)*(scenInfoSize+gameDir.size()) );
				for( int i = 1; i <= gameDir.size(); ++i, ++scenInfoSize)
				{
					char	txtFileName[20];
					scenInfoArray[scenInfoSize].file_name = gameDir[i]->name;    // keep the pointers to the file name string
					scenInfoArray[scenInfoSize].dir_id    = dirId;

					{
						FileSystem::change_file_ext( txtFileName, gameDir[i]->name, "SCT" );

						String str;
						str  = DIR_SCENARIO_PATH(dirId);
						str += txtFileName;

						FileTxt fileTxtScen(str);

						//---- get the name of the scenario ----//

						fileTxtScen.read_line( scenInfoArray[scenInfoSize].scen_name, ScenInfo::SCEN_NAME_LEN );

						//---- get the difficulty level and score bonus ----//

						fileTxtScen.get_token();		// skip "Difficulty:"
						scenInfoArray[scenInfoSize].goal_difficulty = (short) fileTxtScen.get_num();

						fileTxtScen.get_token();		// skip "Bonus:"
						scenInfoArray[scenInfoSize].goal_score_bonus = (short) fileTxtScen.get_num();

						// Get the internal name from the header for player stats tracking
						{
							playerStats.load_player_stats(true);
							String path;
							path = DIR_SCENARIO_PATH(dirId);
							path += gameDir[i]->name;
							PlayStatus status = playerStats.get_scenario_play_status(gameDir[i]->name);
							scenInfoArray[scenInfoSize].play_status = static_cast<int>(status);
						}
					}
				}
			}
		}
	}

	if( scenInfoSize == 0 )
	{
		box.msg( "Sorry, there is no scenario in the game directory." );
		return 0;
	}

	//-------- sort by difficulty ---------- //

	qsort(scenInfoArray, scenInfoSize, sizeof(ScenInfo), sort_scenario_func);

	//-------- select and run a scenario --------//

	int rc = select_scenario( scenInfoSize, scenInfoArray );

	if( rc )
		run_scenario( scenInfoArray+rc-1 );

	//-------------------------------------------//

	mem_del(scenInfoArray);

	return rc;
}
//------------ End of function Game::select_run_scenario -----------//


//---------- Begin of function Game::run_scenario ----------//
//
// <ScenInfo*> scenInfo - the pointer to ScenInfo of the selected scenario
//
int Game::run_scenario(ScenInfo* scenInfo)
{
	String str;

	str  = DIR_SCENARIO_PATH(scenInfo->dir_id);
	str += scenInfo->file_name;

	if( FileSystem::is_file_exist(str) )
	{
		// ###### begin Gilbert 1/11 #########//
		// save the name in the config
		char playerName[HUMAN_NAME_LEN+1];
		strcpy(playerName, config.player_name);
		// ###### end Gilbert 1/11 #########//

		if( SaveGameProvider::load_scenario(str) > 0 )
		{
			ConfigAdv backup;
			if( config_adv.scenario_config )
			{
				String str2;
				str2  = DIR_SCENARIO_PATH(scenInfo->dir_id);
				str2 += "config.txt";

				backup = config_adv;
				config_adv.load(str2);
			}

			init_scenario_var(scenInfo);

			// ##### begin Gilbert 1/11 #######//
			// set the nation name of the player
			if( nation_array.player_recno )
			{
				(~nation_array)->nation_name_id = -nation_array.player_recno;
				nation_array.set_human_name( nation_array.player_recno, playerName);
			}
			strcpy(config.player_name, playerName);
			// ##### end Gilbert 1/11 #######//

			playerStats.set_scenario_play_status(scenInfo->file_name, PlayStatus::PLAYED);

			battle.run_loaded();

			if( config_adv.scenario_config )
				config_adv = backup;
		}
		else
		{
			box.msg(GameFile::status_str());
		}
		game.deinit();
		return 1;
	}

	return 0;
}
//------------ End of function Game::run_scenario -----------//


//-------- Start of function init_scenario_var ----------//
//
static void init_scenario_var(ScenInfo* scenInfo)
{
	//-------- init config var ----------//

	config.king_undie_flag 	  = 0;
	config.disable_ai_flag 	  = 0;
	config.fast_build		  	  = 0;
	config.show_ai_info	  	  = 0;
	config.show_all_unit_icon = 0;
	config.show_unit_path	  = 0;

	sys.set_speed(9, COMMAND_AUTO);

	//------ reset the goal deadline -------//

	info.goal_deadline = date.julian( date.year(info.game_date)+config.goal_year_limit,
								date.month(info.game_date),
								date.day(info.game_date) );

	info.goal_difficulty  = scenInfo->goal_difficulty;
	info.goal_score_bonus = scenInfo->goal_score_bonus;

	info.total_play_time  = 0;

	//------ reset vars in NationBase ---------//

	Nation* playerNation = ~nation_array;		// we reset a whole block of vars in NationBase which are related to financial info

	playerNation->cheat_enabled_flag = 0;

	//------- reset financial vars --------//

	int resetSize = (char*) playerNation->relation_array
						 - (char*)(&(playerNation->cur_year_profit));

	memset( &(playerNation->cur_year_profit), 0, resetSize );

	//------- reset statistic vars -------//

	resetSize = (char*)(&(playerNation->own_firm_destroyed))
						 - (char*)(&(playerNation->enemy_soldier_killed))
						 + sizeof(playerNation->own_firm_destroyed);

	memset( &(playerNation->enemy_soldier_killed), 0, resetSize );

	//--------- update statistic ---------//

	nation_array.update_statistic();

	//------- reset display mode --------//

	sys.view_mode = MODE_NORMAL;

	world.map_matrix->map_mode   = MAP_MODE_TERRAIN;
	world.map_matrix->power_mode = 0;

	//------------ reset news ------------//

	news_array.reset();

	//------ fix firm_build_id problem -----//

	Firm* firmPtr;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->firm_id != FIRM_MONSTER )
			continue;

		int monsterId = ((FirmMonster*)firmPtr)->monster_id;

		firmPtr->firm_build_id = firm_res[FIRM_MONSTER]->get_build_id( monster_res[monsterId]->firm_build_code );
	}
}
//--------- End of function init_scenario_var -----------//



//-------- Start of function init_scenario_var ----------//
//
int sort_scenario_func(const void *arg1, const void *arg2)
{
	return ((ScenInfo *)arg1)->goal_difficulty - ((ScenInfo *)arg2)->goal_difficulty;
}
//-------- End of function init_scenario_var ----------//


