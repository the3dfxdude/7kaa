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

//Filename    : OBATTLE.CPP
//Description : Battle Object

#include <OSYS.h>
#include <OWORLD.h>
#include <OPOWER.h>
#include <OCONFIG.h>
#include <OSPY.h>
#include <OMOUSE.h>
#include <OMUSIC.h>
#include <OSITE.h>
#include <ORACERES.h>
#include <OGODRES.h>
#include <OTECHRES.h>
#include <OCONFIG.h>
#include <OREMOTE.h>
#include <OTOWN.h>
#include <OFIRM.h>
#include <ONEWS.h>
#include <ONATION.h>
#include <OMONSRES.h>
#include <OWALLRES.h>
#include <OINFO.h>
#include <OUNITALL.h>
#include <OGAME.h>
#include <OBATTLE.h>
#include <OMOUSECR.h>
#include <vga_util.h>
#include <CmdLine.h>
#include <FilePath.h>
#include <ConfigAdv.h>

//---------- define static functions -------------//

static int is_space(int xLoc1, int yLoc1, int xLoc2, int yLoc2, char mobileType);
static char random_race();
static char random_race_time();

//-------- Begin of function Battle::init --------//
//
void Battle::init()
{
}
//-------- End of function Battle::init --------//


//-------- Begin of function Battle::deinit --------//
//
void Battle::deinit()
{
}
//-------- End of function Battle::deinit --------//


//-------- Begin of function Battle::run --------//
//
// <int> mpGame - whether this is a multiplayer game or not.
//
void Battle::run(NewNationPara *mpGame, int mpPlayerCount)
{
	int oldCursor = mouse_cursor.get_icon();
	mouse_cursor.set_icon(CURSOR_WAITING);

#ifdef DEBUG
	debug_sim_game_type = (misc.is_file_exist("sim.sys")) ? 2 : 0;
	if(debug_sim_game_type)
	{
		run_sim();
		return;
	}
#endif

	// ####### begin Gilbert 24/10 #######//
	//-- random seed is initalized at connecting multiplayer --//
	//if( !mpGame )
	//	info.init_random_seed(0);
	// ####### end Gilbert 24/10 #######//

	//----------- save the current seed for generating map -----------//
	#ifdef DEBUG2
		File seedFile;
		char *chPtr = misc.format(misc.get_random_seed());
		seedFile.file_create("mapseed.rs");
		seedFile.file_write(chPtr, strlen(chPtr));
		seedFile.file_close();
	#endif

	world.generate_map();

	//------- create player nation --------//

	if( mpGame )
	{
		for( int i = 0; i < mpPlayerCount; ++i )
		{
			int nationRecno = nation_array.new_nation(mpGame[i]);
			if( nationRecno != mpGame[i].nation_recno )
				err.run( "Unexpected nation recno created" );
			nation_array.set_human_name( nationRecno, mpGame[i].player_name );
		}
	}
	else if( game.game_mode != GAME_DEMO )
	{
		// if config.race_id == 0, select a random race, but don't call misc.random
		int nationRecno = nation_array.new_nation( NATION_OWN,
								config.race_id ? config.race_id : random_race_time(),
								config.player_nation_color );

		nation_array.set_human_name( nationRecno, config.player_name );
	}

	//--------- create ai nations --------//

	if( mpGame )
	{
		int aiToCreate = config.ai_nation_count;
		if( aiToCreate + mpPlayerCount > MAX_NATION )
			aiToCreate = MAX_NATION - mpPlayerCount;
		err_when( aiToCreate < 0 );
		create_ai_nation(aiToCreate);
	}
	else if( game.game_mode == GAME_DEMO )
	{
		create_ai_nation(config.ai_nation_count+1); // no human player
	} else {
		create_ai_nation(config.ai_nation_count);
	}

	//------ create pregame objects ------//

	create_pregame_object();

	//------- update nation statistic -------//

	nation_array.update_statistic();

	//--- highlight the player's town in the beginning of the game ---//

	Town* townPtr;

	for( int i=1 ; i<=town_array.size() ; i++ )
	{
		townPtr = town_array[i];

		if( townPtr->nation_recno == nation_array.player_recno )
		{
			world.go_loc( townPtr->loc_x1, townPtr->loc_y1 );
			break;
		}
	}

	//---- reset config parameter ----//

	if( !remote.is_enable() && cmd_line.game_speed >= 0 )
		sys.set_speed(cmd_line.game_speed, COMMAND_AUTO);
	else
		sys.set_speed(12, COMMAND_AUTO);

	//---- reset cheats ----//

	config.fast_build = 0;
	config.king_undie_flag = sys.testing_session && !mpGame;
	config.blacken_map = 1;
	config.disable_ai_flag = 0;

	if( sys.testing_session )
		config.show_unit_path = 3;

	if( game.game_mode == GAME_DEMO )
	{
		// observation mode
		world.unveil(0, 0, MAX_WORLD_X_LOC-1, MAX_WORLD_Y_LOC-1);
		world.visit(0, 0, MAX_WORLD_X_LOC-1, MAX_WORLD_Y_LOC-1, 0, 0);

		config.blacken_map = 0;
		config.fog_of_war = 0;
	}

	// ######## begin Gilbert 11/11 #######//
	// enable tech and god, useful for multi-player
#if(0)
	for( i = 1; i < nation_array.size(); ++i )
	{
		if( !nation_array.is_deleted(i) && !nation_array[i]->is_ai() )
		{
			tech_res.inc_all_tech_level(i);
			god_res.enable_know_all(i);
		}
	}
#endif
	// ######## end Gilbert 11/11 #######//

	//------- enable/disable sound effects -------//

	int songId;
	if( nation_array.player_recno && (~nation_array)->race_id <= 7 )
		songId = (~nation_array)->race_id+1;
	else
		songId = music.random_bgm_track();
	music.play(songId, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );

	mouse_cursor.restore_icon(oldCursor);

	//--- give the control to the system main loop, start the game now ---//

	sys.run();
}
//--------- End of function Battle::run ---------//


//-------- Begin of function Battle::run_sim --------//
// run simulation
//
#ifdef DEBUG
void Battle::run_sim()
{
	err_when(!debug_sim_game_type);

	info.disp_panel();
	//info.init_random_seed(0);
	//info.init_random_seed(869451513); // for testing marine
	info.init_random_seed(869639665); // for testing marine

	//info.init_random_seed(866608391);
	//info.init_random_seed(866621716);
	//info.init_random_seed(867299236);

	world.generate_map();

	//--------- refresh world ---------//
	world.refresh();
	vga_util.blt_buf(0, 0, VGA_WIDTH-1, VGA_HEIGHT-1);
	world.paint();

	//------- create player nation --------//

	// if config.race_id == 0, select a random race, but don't call misc.random
	nation_array.new_nation( NATION_OWN,
		config.race_id ? config.race_id : random_race_time(),
		config.player_nation_color );

	//--------- create ai nations --------//
	create_ai_nation(config.ai_nation_count);

	//------ create pregame objects ------//
	create_pregame_object();

	//--- highlight the player's town in the beginning of the game ---//
	Town* townPtr;
	for( int i=1 ; i<=town_array.size() ; i++ )
	{
		townPtr = town_array[i];

		if( townPtr->nation_recno == nation_array.player_recno )
		{
			world.go_loc( townPtr->loc_x1, townPtr->loc_y1 );
			break;
		}
	}

	//-*************** create units, objects *****************-//
	int maxNationCount = 2;
	int unitId = UNIT_DRAGON;
	int nationCount;
	SpriteInfo *spriteInfo;
	char teraMask;
	int unitRecno, x, y, xLoc, yLoc;

	//--------- create dragon ---------//
	spriteInfo = sprite_res[unit_res[unitId]->sprite_id];
	teraMask = UnitRes::mobile_type_to_mask(unit_res[unitId]->mobile_type);
	for(nationCount=1; nationCount<=maxNationCount; nationCount++)
	{
		xLoc = 0;
		yLoc = MIN(20*nationCount, 180);

		for(int createCount=0; createCount<10; createCount++, xLoc+=4) // createCount<50
		{
			for(y=0; y<4; y+=2)
			{
				for(x=0; x<4; x+=2)
				{
					unitRecno = unit_array.add_unit(unitId, nationCount, RANK_SOLDIER, 100, xLoc+x, yLoc+y);
					unit_array[unitRecno]->set_combat_level(100);
					((UnitGod*)unit_array[unitRecno])->god_id = 1;
				}
			}
		}
	}

	//--------- create marine units ---------//
	unitId = UNIT_CARAVEL;
	spriteInfo = sprite_res[unit_res[unitId]->sprite_id];
	teraMask = UnitRes::mobile_type_to_mask(unit_res[unitId]->mobile_type);
	for(nationCount=1; nationCount<=maxNationCount; nationCount++)
	{
		for(int t=0; t<30; t++)
		{
			xLoc=0;
			yLoc=0;
			if(world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
				MAX_WORLD_Y_LOC-1, spriteInfo->loc_width*4, spriteInfo->loc_height*4,
				MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, 0, 0, teraMask))
			{
				if(xLoc%2)
					xLoc++;
				if(yLoc%2)
					yLoc++;

				for(y=0; y<2; y++)
				{
					for(x=0; x<2; x++)
					{
						//world.get_loc(startXLoc, startYLoc)->can_move(unit_res[unit_id]->mobile_type)
						unitRecno = unit_array.add_unit(unitId, nationCount, RANK_SOLDIER, 100, xLoc+x*2, yLoc+y*2);
						unit_array[unitRecno]->set_combat_level(100);
					}
				}
			}
		}
	}

	//--------- create land units ---------//
	for(nationCount=1; nationCount<=maxNationCount; nationCount++)
	{
		for(unitId=UNIT_NORMAN; unitId<=UNIT_JAPANESE; unitId++)
		{
			spriteInfo = sprite_res[unit_res[unitId]->sprite_id];
			teraMask = UnitRes::mobile_type_to_mask(unit_res[unitId]->mobile_type);
			for(int t=0; t<5; t++)
			{
				xLoc=0;
				yLoc=0;
				if(world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
					MAX_WORLD_Y_LOC-1, spriteInfo->loc_width*2, spriteInfo->loc_height*2,
					MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, 0, 0, teraMask))
				{
					for(y=0; y<2; y++)
					{
						for(x=0; x<2; x++)
						{
							//world.get_loc(startXLoc, startYLoc)->can_move(unit_res[unit_id]->mobile_type)
							unitRecno = unit_array.add_unit(unitId, nationCount, RANK_SOLDIER, 100, xLoc+x, yLoc+y);
							unit_array[unitRecno]->set_combat_level(100);
						}
					}
				}
			}
		}
	}

	//-- enable power after the game objets has been initialized --//
	power.enable();      // enable power, which handle mouse inputs

	//--- give the control to the system main loop, start the game now ---//
	sys.run();
}
#endif
//--------- End of function Battle::run_sim ---------//


//-------- Begin of function Battle::create_ai_nation --------//
//
// Create AI nations.
//
void Battle::create_ai_nation(int aiNationCount)
{
	int raceId;

	for( int i=0 ; i<aiNationCount ; i++ )
	{
		err_when( nation_array.size() == MAX_NATION );

		if( config.random_start_up )
			raceId = random_race();
		else
			raceId = nation_array.random_unused_race();

		err_when( raceId < 1 || raceId > MAX_RACE );

		int nationRecno;
		nationRecno = nation_array.new_nation( NATION_AI, raceId, nation_array.random_unused_color() );     // 2nd parameter = the race id., 3rd parameters = color scheme id.
	}
}
//--------- End of function Battle::create_ai_nation ---------//


//-------- Begin of function Battle::create_pregame_object --------//
//
// Initialize pre-game objects - towns, sites, independent towns.
//
void Battle::create_pregame_object()
{
	#define CREATE_UNIT_AREA_WIDTH     16
	#define CREATE_UNIT_AREA_HEIGHT    16

	// ###### begin Gilbert 24/10 ######//
	const int dispProgress = 1;
	const int maxGenMapSteps = 100;
	const int newWorldSection = 1;
	vga_front.unlock_buf();

	int curGenMapSteps = 0;
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, newWorldSection);
		vga_front.unlock_buf();
	}
	// ###### end Gilbert 24/10 ######//

	//------- create nation and units --------//

	int 		nationRecno, unitId, rankId, xLoc, yLoc, townRecno;
	int		kingUnitRecno;
	int		noSpaceFlag=0;
	Nation*  nationPtr;

	for( nationRecno=1 ; nationRecno<=nation_array.size() ; nationRecno++ )
	{
		if( nation_array.is_deleted(nationRecno) )
			continue;

		nationPtr = nation_array[nationRecno];

		//--------- create town -----------//

		townRecno = create_town( nationRecno, nationPtr->race_id, xLoc, yLoc );

		if( !townRecno )
		{
			noSpaceFlag = 1;
			break;
		}

		//------- create military camp -------//

		Town* townPtr = town_array[townRecno];

		int firmRecno = firm_array.build_firm(townPtr->loc_x1+6, townPtr->loc_y1,
							 nationRecno, FIRM_CAMP, race_res[nationPtr->race_id]->code);

		if( !firmRecno )
		{
			noSpaceFlag = 1;
			break;
		}

		firm_array[firmRecno]->complete_construction();

		//--------- create units ----------//

		unitId = race_res[nationPtr->race_id]->basic_unit_id;

		kingUnitRecno = create_unit(townRecno, unitId, RANK_KING);

		if( kingUnitRecno )
		{
			nation_array[nationRecno]->set_king(kingUnitRecno, 1);		// 1-this is the first king of the nation
			firm_array[firmRecno]->assign_overseer(kingUnitRecno);	// assign the king as the overseer of the command base
		}
		else
		{
			noSpaceFlag = 1;
			break;
		}

		//----- create skilled units if config.random_start_up is 1 -----//

		if( config.random_start_up )
		{
			int createCount = (50-townPtr->population)/3;		// the less population the villager has the more mobile units will be created

			for( int i=0 ; i<createCount ; i++ )
			{
				if( misc.random(2)==0 )
					unitId = race_res[nationPtr->race_id]->basic_unit_id;
				else
					unitId = race_res[ random_race() ]->basic_unit_id;

				if( misc.random(3)==0 )
					rankId = RANK_GENERAL;
				else
					rankId = RANK_SOLDIER;

				if( !create_unit(townRecno, unitId, rankId) )
					break;
			}
		}

		//------ create mines near towns in the beginning -----//

		if( config.start_up_has_mine_nearby && !nationPtr->is_ai() )
			site_array.create_raw_site(0, townRecno);
	}

	//--- if there is no space for creating new town/firm or unit, delete the unprocessed nations ---//

	if( noSpaceFlag )
	{
		for( ; nationRecno<=nation_array.size() ; nationRecno++ )
			nation_array.del_nation(nationRecno);		// no space for creating a town for the nation, so we have to delete the nation
	}

	// ###### begin Gilbert 24/10 ########//
	curGenMapSteps += 10;			// 10
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, newWorldSection);
		vga_front.unlock_buf();
	}
	// ###### end Gilbert 24/10 ######//

	//---- init the type of active monsters in this game ----//

	monster_res.init_active_monster();
	// ###### begin Gilbert 24/10 ########//
	curGenMapSteps += 5;			// 15
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, newWorldSection);
		vga_front.unlock_buf();
	}
	// ###### end Gilbert 24/10 ######//

	//------ create independent towns -------//

	//### begin alex 27/8 ###//
	int startUpIndependentTown = config.start_up_independent_town;
	//int startUpRawSite = config.start_up_raw_site;
	int startUpMonsterFirm = 10;
	int i, j, raceId;

	site_array.generate_raw_site(config.start_up_raw_site);
	// ###### begin Gilbert 24/10 ########//
	curGenMapSteps += 10;			// 25
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, newWorldSection);
		vga_front.unlock_buf();
	}

	int targetStep = maxGenMapSteps;
	// ###### end Gilbert 24/10 ######//

	int maxLoopCount = startUpIndependentTown + startUpMonsterFirm;

	for(j=0, i=1; j<maxLoopCount; j++, i++)
	{
		if(startUpIndependentTown)
		{
			//------ create independent towns -------//
			raceId = config_adv.race_random_list[i%config_adv.race_random_list_max];
			if(!create_town( 0, raceId, xLoc, yLoc ) )
			{
				startUpIndependentTown = 0;
				break;
			}
			// ##### begin Gilbert 24/10 #######//
			else
				startUpIndependentTown--;
			// ###### end Gilbert 24/10 ########//
		}

		if(startUpMonsterFirm)
		{
			//------- create mosnters --------//
			if(config.monster_type != OPTION_MONSTER_NONE)
			{
				monster_res.generate(1);
				startUpMonsterFirm--;
			}
			else
				startUpMonsterFirm = 0;
		}

		if(!(startUpIndependentTown+startUpMonsterFirm))
			break;

		// ###### begin Gilbert 24/10 ########//
		if( dispProgress )
		{
			vga_front.lock_buf();
			game.disp_gen_map_status( curGenMapSteps + j*(targetStep-curGenMapSteps)/maxLoopCount, maxGenMapSteps, newWorldSection);
			vga_front.unlock_buf();
		}
		// ###### end Gilbert 24/10 ######//
	}
	//#### end alex 27/8 ####//

	// ###### begin Gilbert 24/10 ########//
	// finish
	curGenMapSteps = targetStep;		// 100
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, newWorldSection);
		vga_front.unlock_buf();
	}
	// ###### end Gilbert 24/10 ######//

	vga_front.lock_buf();
}
//--------- End of function Battle::create_pregame_object ---------//


//-------- Begin of function Battle::run_loaded --------//
//
void Battle::run_loaded()
{
	//-------- play music ---------//

	char kingRace = 0;

	if( !nation_array.is_deleted(nation_array.player_recno) )
	{
		if( !unit_array.is_deleted((~nation_array)->king_unit_recno) )
			// if there is king in the nation take his race
			kingRace = unit_array[(~nation_array)->king_unit_recno]->race_id;
		else
			// if king is killed, get the original nation
			kingRace = (~nation_array)->race_id;
	}

	int songId = kingRace <= 7 ? kingRace+1 : music.random_bgm_track();
	music.play(songId, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );

	//--- give the control to the system main loop, start the game now ---//

	sys.run(1);
}
//--------- End of function Battle::run_loaded ---------//


//-------- Begin of function Battle::run_replay --------//
//
void Battle::run_replay()
{
	NewNationPara *mpGame = (NewNationPara *)mem_add(sizeof(NewNationPara)*MAX_NATION);
	int mpPlayerCount = 0;
	FilePath full_path(sys.dir_config);
	Config tmpConfig = config;

	full_path += "NONAME.RPL";
	if( full_path.error_flag )
		return;

	if( !remote.init_replay_load(full_path, mpGame, &mpPlayerCount) )
		return;

	game.init();
	game.game_mode = GAME_DEMO;
	game.game_has_ended = 1;
	battle.run(mpGame, mpPlayerCount);
	mem_del(mpGame);
	remote.deinit();
	game.deinit();
	config = tmpConfig;
}
//--------- End of function Battle::run_replay ---------//


//-------- Begin of function Battle::run_test --------//
//
void Battle::run_test()
{
	info.disp_panel();

	info.init_random_seed(153542);

	world.generate_map();

	//--------- refresh world ---------//

	world.refresh();

	vga_util.blt_buf(0, 0, VGA_WIDTH-1, VGA_HEIGHT-1);

	world.paint();

	//-------- create nation and units --------//

	nation_array.new_nation( NATION_OWN, 1, 1 );		// 2nd parameter = the race id.	3rd parameters = color scheme id.
	nation_array.new_nation( NATION_AI , 2, 2 );		// 2nd parameter = the race id.	3rd parameters = color scheme id.

	tech_res.inc_all_tech_level(1);		// set all tech of nation 1 to level 1
	tech_res.inc_all_tech_level(2);		// set all tech of nation 2 to level 1

	create_test_unit(1);
	create_test_unit(2);

	//-- enable power after the game objets has been initialized --//

	power.enable();		// enable power, which handle mouse inputs

	//--- give the control to the system main loop, start the game now ---//

	sys.run();
}
//--------- End of function Battle::run_test ---------//


//-------- Begin of function Battle::create_test_unit --------//
//
void Battle::create_test_unit(int nationRecno)
{
	int 			x, y, unitId, xLoc, yLoc;
	SpriteInfo* spriteInfo;

	for( unitId=1 ; unitId<=MAX_UNIT_TYPE ; unitId++ )
	{
		if( unitId == UNIT_CARAVAN )		// caravan cannot be created independently without market places
			continue;

		xLoc=0;
		yLoc=0;

		spriteInfo = sprite_res[ unit_res[unitId]->sprite_id ];
		char teraMask = UnitRes::mobile_type_to_mask(unit_res[unitId]->mobile_type);

		if( world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
			 MAX_WORLD_Y_LOC-1, spriteInfo->loc_width*4, spriteInfo->loc_height*4,
			 MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, 0, 0, teraMask) )
		{
			for( y=spriteInfo->loc_height ; y<spriteInfo->loc_height*4 ; y+=spriteInfo->loc_height*2 )
			{
				for( x=spriteInfo->loc_width ; x<spriteInfo->loc_width*4 ; x+=spriteInfo->loc_width*2 )
				{
					// force the location to be even number
					int unitRecno = unit_array.add_unit( unitId, nationRecno, RANK_SOLDIER, 100, (xLoc+x)& ~1, (yLoc+y) & ~1 );
					Unit* unitPtr = unit_array[unitRecno];

					unitPtr->set_combat_level(100);
					if( unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_MONSTER )
					{
						// normally set by the monster firm, but there is no monster firm
						unitPtr->set_monster_id(monster_res.get_monster_by_unit_id(unitId)->monster_id);
					}
				}
			}
		}
	}
}
//--------- End of function Battle::create_test_unit ---------//


//-------- Begin of function Battle::create_town --------//
//
// <int> nationRecno = the nation recno of the town
// <int> raceId      = the race id. of the town
//
// <int&> xLoc = for the starting location of the town
// <int&> yLoc = for the starting location of the town
//
// return: <int> townRecno - >0  the recno of the town created
//                           ==0 no town created
//
int Battle::create_town(int nationRecno, int raceId, int& xLoc, int& yLoc)
{
	//------- locate for a space to build the town ------//

	if( !town_array.think_town_loc(MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, xLoc, yLoc) )
		return 0;

	//--------------- create town ---------------//

	int townRecno = town_array.add_town(nationRecno, raceId, xLoc, yLoc);

	Town* townPtr = town_array[townRecno];

	//--------- no. of mixed races ---------//

	if( nationRecno )
	{
		int initPop;

		if( config.random_start_up )
			initPop = 25 + misc.random(26);		// 25 to 50
		else
			initPop = 40;

		Town* townPtr = town_array[townRecno];

		townPtr->init_pop( raceId, initPop, 100, 0, 1 );		// 100-startup loyalty, last 1-first initialization at the beginning of the game
	}
	else
	{
		int mixedRaceCount;

		if( nationRecno )
			mixedRaceCount = 1;
		else
			mixedRaceCount= misc.random(3)+1;		// 1 to 3 mixed races

		int curPop, totalPop=0, townResistance;

		for( int i=0 ; i<mixedRaceCount ; i++ )
		{
			if(totalPop>=MAX_TOWN_POPULATION)
				break;

			townResistance = town_array.independent_town_resistance();

			if( i==0 )
			{
				curPop = 15/mixedRaceCount + misc.random(15/mixedRaceCount);
				if(curPop>=MAX_TOWN_POPULATION)
					curPop = MAX_TOWN_POPULATION;

				err_when(curPop==0);
				townPtr->init_pop( raceId, curPop, townResistance, 0, 1 ); 	// last 1-first initialization at the beginning of the game
				totalPop += curPop;
			}
			else
			{
				curPop = 10/mixedRaceCount + misc.random(10/mixedRaceCount);
				if(curPop>=MAX_TOWN_POPULATION-totalPop)
					curPop = MAX_TOWN_POPULATION-totalPop;

				err_when(curPop==0);
				townPtr->init_pop( random_race(), curPop, townResistance, 0, 1 );
				totalPop += curPop;
			}
		}
	}

	//---------- set town layout -----------//

	townPtr->auto_set_layout();

	return townRecno;
}
//--------- End of function Battle::create_town ---------//


//-------- Begin of function Battle::create_unit --------//
//
// Add a specific no. of units within a given area
//
// <int> townRecno   - create units around this town.
// <int> unitId      - id. of the units to be added
// <int> rankId      - rank id. of the unit
//
// return: <int> recno of the unit created.
//
int Battle::create_unit(int townRecno, int unitId, int rankId)
{
	SpriteInfo* spriteInfo = sprite_res[ unit_res[unitId]->sprite_id ];
	Town* 		townPtr = town_array[townRecno];

	//------ locate space for the unit ------//

	int xLoc = townPtr->loc_x1;
	int yLoc = townPtr->loc_y1;

	if( !world.locate_space( &xLoc, &yLoc, xLoc+STD_TOWN_LOC_WIDTH-1, yLoc+STD_TOWN_LOC_HEIGHT-1,
									 spriteInfo->loc_width, spriteInfo->loc_height ) )
	{
		return 0;
	}

	//---------- create the unit --------//

	int unitLoyalty = 80 + misc.random(20);

	int unitRecno = unit_array.add_unit( unitId, townPtr->nation_recno, rankId, unitLoyalty, xLoc, yLoc );

	if( !unitRecno )
		return 0;

	Unit* unitPtr = unit_array[unitRecno];

	//----------- set skill -------------//

	switch( rankId )
	{
		case RANK_KING:
			unitPtr->skill.set_skill(SKILL_LEADING);
			unitPtr->skill.skill_level = 100;
			unitPtr->set_combat_level(100);
			break;

		case RANK_GENERAL:
			unitPtr->skill.set_skill(SKILL_LEADING);
			unitPtr->skill.skill_level = 40 + misc.random(50);		// 40 to 90
			unitPtr->set_combat_level(30 + misc.random(70));		// 30 to 100
			break;

		case RANK_SOLDIER:
		{
			int skillId = misc.random(MAX_TRAINABLE_SKILL)+1;
			int spyFlag = 0;

			if( skillId == SKILL_SPYING )
			{
				spyFlag = 1;

				unitPtr->set_combat_level(10+misc.random(10));
			}
			else
			{
				unitPtr->skill.skill_id 	= skillId;
				unitPtr->skill.skill_level = 30+misc.random(70);

				if( skillId == SKILL_LEADING )
					unitPtr->set_combat_level(30+misc.random(70));
				else
					unitPtr->set_combat_level(10+misc.random(10));

				if( misc.random(5)==0 )
					spyFlag = 1;
			}

			if( spyFlag )
			{
				int spySkill = 20 + misc.random(80);		// 20 to 100
				unitPtr->spy_recno = spy_array.add_spy(unitRecno, spySkill);
			}

			break;
		}

		default:
			err_here();
	}

	return unitRecno;
}
//--------- End of function Battle::create_unit ---------//


//-------- Begin of static function is_space --------//
//
// Check whether all locations in the given area are space
//
// return : <int> 1 - all are space
//                0 - not all are space
//
static int is_space(int xLoc1, int yLoc1, int xLoc2, int yLoc2, char mobileType)
{
   int       xLoc, yLoc;
   Location* locPtr;

   for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
   {
      locPtr = world.get_loc(xLoc1, yLoc);

      for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
      {
			if( !locPtr->can_move(mobileType) )
				return 0;
      }
   }

   return 1;
}
//--------- End of static function is_space ---------//


//-------- Begin of static function random_race --------//
//
// Uses misc.random() for random race
//
static char random_race()
{
	int num = misc.random(config_adv.race_random_list_max);
	return config_adv.race_random_list[num];
}
//--------- End of static function random_race ---------//


//-------- Begin of static function random_race_time --------//
//
// Uses current time for random race
//
static char random_race_time()
{
	int num = misc.get_time();
	return config_adv.race_random_list[num%config_adv.race_random_list_max];
}
//--------- End of static function random_race_time ---------//
