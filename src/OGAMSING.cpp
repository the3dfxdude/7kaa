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

//Filename    : OGAMSING.CPP
//Description : Single player game interface

#include <OVGA.h>
#include <vga_util.h>
#include <OSYS.h>
#include <OMOUSE.h>
#include <OFONT.h>
#include <OIMGRES.h>
#include <ORACERES.h>
#include <OINFO.h>
#include <OBUTT3D.h>
#include <OGET.h>
#include <OBATTLE.h>
#include <OGAME.h>
#include <OCONFIG.h>
#include <OMUSIC.h>
#include <OBUTTCUS.h>
#include <OCOLTBL.h>
#include <OGETA.h>
#include <stdlib.h>

//---------- Define option modes --------//

enum { OPTION_BASIC,
		 OPTION_ADVANCED,
		 OPTION_ADVANCE2,
		 OPTION_GOAL,
	  };

static char option_mode;

//--------- Define constant ----------//

enum { BASIC_OPTION_X_SPACE = 78,
		 BASIC_OPTION_HEIGHT = 32 };

enum { COLOR_OPTION_X_SPACE = 35,
		 COLOR_OPTION_HEIGHT = 32 };

enum { ADVANCED_OPTION_X_SPACE = 89,
		 ADVANCED_OPTION_Y_SPACE = 34,
		 ADVANCED_OPTION_HEIGHT  = 32  };

static char race_table[MAX_RACE] =		// race translation table
#if(MAX_RACE == 10)
{
	RACE_CHINESE, RACE_EGYPTIAN, RACE_GREEK, RACE_JAPANESE, RACE_MAYA,
	RACE_INDIAN, RACE_NORMAN, RACE_PERSIAN, RACE_VIKING, RACE_ZULU
};
#else
{
	RACE_CHINESE, RACE_GREEK, RACE_JAPANESE, RACE_MAYA,
	RACE_PERSIAN, RACE_NORMAN, RACE_VIKING
};
#endif

static char reverse_race_table[MAX_RACE] =		// race translation table
#if(MAX_RACE == 10)
{
	6, 4, 2, 8, 7, 0, 3, 1, 5, 9
};
#else
{
	5, 3, 1, 6, 4, 0, 2
};
#endif

//------- Declare static functions -------//

static int	select_option();

//-------- Begin of function Game::single_player_game --------//
//
// <int> noAI - if there should be no AI in the game.
//
void Game::single_player_game(int noAI)
{
	sys.is_mp_game = 0;

	if( !select_option() )
		return;

	// ------ attempt to save the config -------//
	config.save("CONFIG.DAT");

	//------ start single player game ----------//

	init();
	battle.run(0);			// 0-not multiplayer game
	deinit();
}
//--------- End of function Game::single_player_game ---------//



// define bit flag for refreshFlag
static void disp_virtual_button(ButtonCustom *, int);
static void disp_virtual_tick(ButtonCustom *, int);

#define SGOPTION_PAGE           0x40000000
#define SGOPTION_RACE           0x00000001
#define SGOPTION_COLOR          0x00000002
#define SGOPTION_AI_NATION      0x00000004
#define SGOPTION_DIFFICULTY     0x00000008
#define SGOPTION_TERRAIN        0x00000010
#define SGOPTION_LAND_MASS      0x00000020
#define SGOPTION_NAME_FIELD     0x00000040
#define SGOPTION_MAP_ID         0x00000080
#define SGOPTION_EXPLORED       0x00000100
#define SGOPTION_FOG            0x00000200
#define SGOPTION_TREASURE       0x00000400
#define SGOPTION_AI_TREASURE    0x00000800
#define SGOPTION_AI_AGGRESSIVE  0x00001000
#define SGOPTION_FRYHTANS       0x00002000
#define SGOPTION_RANDOM_STARTUP 0x00004000
#define SGOPTION_RAW            0x00010000
#define SGOPTION_NEAR_RAW       0x00020000
#define SGOPTION_START_TOWN     0x00040000
#define SGOPTION_TOWN_STRENGTH  0x00080000
#define SGOPTION_TOWN_EMERGE    0x00100000
#define SGOPTION_KINGDOM_EMERGE 0x00200000
#define SGOPTION_RANDOM_EVENT   0x00400000
#define SGOPTION_CLEAR_ENEMY    0x01000000
#define SGOPTION_CLEAR_MONSTER  0x02000000
#define SGOPTION_ENOUGH_PEOPLE  0x04000000
#define SGOPTION_ENOUGH_INCOME  0x08000000
#define SGOPTION_ENOUGH_SCORE   0x10000000
#define SGOPTION_TIME_LIMIT     0x20000000
#define SGOPTION_ALL            0x7fffffff

static int select_option()
{
	const int offsetY = 124;
	char optionMode = OPTION_BASIC;
	char menuTitleBitmap[] = "TOP-NSPG";
	
	Config tempConfig = config;

	// last multi-player game may set ai_nation_count to zero
	if( tempConfig.ai_nation_count < 1 )
		tempConfig.ai_nation_count = MAX_NATION-1;

	// some setting may be modified in the last game
	if( tempConfig.difficulty_level != OPTION_CUSTOM )
		tempConfig.change_difficulty(tempConfig.difficulty_level);

	int i;
	long refreshFlag = SGOPTION_ALL;
	int retFlag = 0;

	// --------- initialize race button group ---------- //

	ButtonCustomGroup raceGroup(MAX_RACE);
	for( i = 0; i < MAX_RACE; ++i )
	{
#if(MAX_RACE == 10)
		raceGroup[i].create(222+(i%5)*BASIC_OPTION_X_SPACE, offsetY+81+(i/5)*BASIC_OPTION_HEIGHT,
			222+(i%5+1)*BASIC_OPTION_X_SPACE-1, offsetY+81+(i/5+1)*BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&raceGroup, race_table[i]), 0, 0);

#define Y_SHIFT_FLAG 1
#else
		raceGroup[i].create(118+i*BASIC_OPTION_X_SPACE, offsetY+103,
			118+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+103+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&raceGroup, race_table[i]), 0, 0);
#define Y_SHIFT_FLAG 0
#endif
	}

	// --------- initialize color button group ---------- //

	ButtonCustomGroup colorGroup(MAX_COLOR_SCHEME);
	for( i = 0; i < MAX_COLOR_SCHEME; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 14
		#else
			#define Y_SHIFT 0
		#endif
		colorGroup[i].create(195+i*COLOR_OPTION_X_SPACE, offsetY+149+Y_SHIFT,
			195+(i+1)*COLOR_OPTION_X_SPACE-1, offsetY+149+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&colorGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	// ---------- initialize ai_nation_count buttons --------//

	ButtonCustom aiNationInc, aiNationDec;
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 14
		#else
			#define Y_SHIFT 0
		#endif
		aiNationInc.create(595, offsetY+149+Y_SHIFT, 
			595+COLOR_OPTION_X_SPACE-1, offsetY+149+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, +1) );
		aiNationDec.create(630, offsetY+149+Y_SHIFT, 
			630+COLOR_OPTION_X_SPACE-1, offsetY+149+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, -1) );
		#undef Y_SHIFT
	}

	// ---------- initialize difficulty_level button group -------//

	ButtonCustomGroup diffGroup(6);
	for( i = 0; i < 6; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 17
		#else
			#define Y_SHIFT 0
		#endif
		diffGroup[i].create( 205+i*BASIC_OPTION_X_SPACE, offsetY+194+Y_SHIFT,
			205+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+194+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&diffGroup, i), 0, 0);
		#undef Y_SHIFT
	}

	// --------- initialize terrain_set button group -------//

	ButtonCustomGroup terrainGroup(3);
	for( i = 0; i < 3; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 17
		#else
			#define Y_SHIFT 0
		#endif
		terrainGroup[i].create(168+i*BASIC_OPTION_X_SPACE, offsetY+258+Y_SHIFT, 
			168+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+258+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&terrainGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	// --------- initialize land_mass button group -------//

	ButtonCustomGroup landGroup(3);
	for( i = 0; i < 3; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 17
		#else
			#define Y_SHIFT 0
		#endif
		landGroup[i].create(439+i*BASIC_OPTION_X_SPACE, offsetY+258+Y_SHIFT, 
			439+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+258+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&landGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	GetA playerNameField;
	// ####### begin Gilbert 3/11 #######//
//	playerNameField.init( 318, offsetY+327, 690, tempConfig.player_name,
//		HUMAN_NAME_LEN, &font_san, 0, 1);
	playerNameField.init( 318, offsetY+322, 690, tempConfig.player_name,
		HUMAN_NAME_LEN, &font_bard, 0, 1);
	// ####### end Gilbert 3/11 #######//

	// --------- initialize info.random_seed field ----------//

	const int mapIdSize = 11;		// enough to hold a dword in decimal
	char mapIdStr[mapIdSize+1];
	info.init_random_seed(0);
	sprintf( mapIdStr,"%d",info.random_seed);
	GetA mapIdField;
#if(defined(SPANISH))
	#define MAPID_X1 586
#elif(defined(FRENCH))
	#define MAPID_X1 572
#else
	#define MAPID_X1 564
#endif
	mapIdField.init( MAPID_X1, offsetY+112, 700, mapIdStr, mapIdSize, &font_san, 0, 1);
#undef MAPID_X1
	// --------- initialize explore_whole_map button group -------//

	ButtonCustomGroup exploreGroup(2);
	for( i = 0; i < 2; ++i )
	{
		exploreGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+103, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+103+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&exploreGroup, 1-i), 0, 0);
	}

	// --------- initialize fog_of_war button group -------//

	ButtonCustomGroup fogGroup(2);
	for( i = 0; i < 2; ++i )
	{
		fogGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+135, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+135+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&fogGroup, 1-i), 0, 0);
	}

	// --------- initialize start_up_cash/start_up_food button group -------//

	ButtonCustomGroup treasureGroup(4);
	for( i = 0; i < 4; ++i )
	{
		treasureGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+167,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+167+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&treasureGroup, i+1), 0, 0);
	}

	// --------- initialize ai_start_up_cash/food button group -------//

	ButtonCustomGroup aiTreasureGroup(4);
	for( i = 0; i < 4; ++i )
	{
		aiTreasureGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+199,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+199+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&aiTreasureGroup, i+1), 0, 0);
	}

	// --------- initialize ai_aggressiveness -------//

	ButtonCustomGroup aiAggressiveGroup(4);
	for( i = 0; i < 4; ++i )
	{
		aiAggressiveGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+231, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+231+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&aiAggressiveGroup, i+1), 0, 0);
	}

	// --------- initialize monster_type -------//

	ButtonCustomGroup monsterGroup(3);
	for( i = 0; i < 3; ++i )
	{
		monsterGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+263,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+263+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&monsterGroup, i), 0, 0);
	}

	// --------- initialize random startup button group -------//

	ButtonCustomGroup randomStartUpGroup(2);
	for( i = 0; i < 2; ++i )
	{
		randomStartUpGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+295, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+295+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&randomStartUpGroup, 1-i), 0, 0);
	}

	//  -------- initialize start_up_raw_site buttons --------- //

	ButtonCustom rawSiteInc, rawSiteDec;
	rawSiteInc.create( 368, offsetY+100, 
		368+COLOR_OPTION_X_SPACE-1, offsetY+100+COLOR_OPTION_HEIGHT-1,
		disp_virtual_button, ButtonCustomPara(NULL,0));
	rawSiteDec.create( 403, offsetY+100, 
		403+COLOR_OPTION_X_SPACE-1, offsetY+100+COLOR_OPTION_HEIGHT-1,
		disp_virtual_button, ButtonCustomPara(NULL,0));

	// --------- initialize start_up_has_mine_nearby button group --------//

	ButtonCustomGroup nearRawGroup(2);
	for( i = 0; i < 2; ++i )
	{
		nearRawGroup[i].create(332+i*BASIC_OPTION_X_SPACE, offsetY+132,
			332+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+132+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&nearRawGroup, 1-i), 0, 0);
	}

	// --------- initialize start_up_independent_town button group --------//

	static short startTownArray[3] = { 7, 15, 30 };

	ButtonCustomGroup townStartGroup(3);
	for( i = 0; i < 3; ++i )
	{
		townStartGroup[i].create(332+i*BASIC_OPTION_X_SPACE, offsetY+164, 
			332+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+164+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townStartGroup, startTownArray[i]), 0, 0);
	}

	// --------- initialize independent_town_resistance button group --------//

	ButtonCustomGroup townResistGroup(3);
	for( i = 0; i < 3; ++i )
	{
		townResistGroup[i].create(332+i*BASIC_OPTION_X_SPACE, offsetY+196, 
			332+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+196+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townResistGroup, i+1), 0, 0);
	}

	// --------- initialize new_independent_town_emerge button group --------//

	ButtonCustomGroup townEmergeGroup(2);
	for( i = 0; i < 2; ++i )
	{
		townEmergeGroup[i].create(332+i*BASIC_OPTION_X_SPACE, offsetY+228,
			332+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+228+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townEmergeGroup, 1-i), 0, 0);
	}

	// --------- initialize new_nation_emerge button group --------//

	ButtonCustomGroup nationEmergeGroup(2);
	for( i = 0; i < 2; ++i )
	{
		nationEmergeGroup[i].create(332+i*BASIC_OPTION_X_SPACE, offsetY+260, 
			332+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+260+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&nationEmergeGroup, 1-i), 0, 0);
	}

	// --------- initialize random_event_frequency button group --------//

	ButtonCustomGroup randomEventGroup(4);
	for( i = 0; i < 4; ++i )
	{
		randomEventGroup[i].create(332+i*BASIC_OPTION_X_SPACE, offsetY+292, 
			332+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+292+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&randomEventGroup, i), 0, 0);
	}

	// ---------- initialize goal buttons ----------//

	ButtonCustom clearEnemyButton, clearMonsterButton, enoughPeopleButton, enoughIncomeButton, enoughScoreButton, timeLimitButton;
	ButtonCustom peopleInc, peopleDec, incomeInc, incomeDec, scoreInc, scoreDec, yearInc, yearDec;

	clearEnemyButton.create( 214, offsetY+145, 214+19, offsetY+145+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 1);
	clearEnemyButton.enable_flag = 0;;
	clearMonsterButton.create( 214, offsetY+178, 214+19, offsetY+178+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 
		tempConfig.goal_destroy_monster);
	enoughPeopleButton.create( 214, offsetY+211, 214+19, offsetY+211+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 
		tempConfig.goal_population_flag);
	enoughIncomeButton.create( 214, offsetY+244, 214+19, offsetY+244+19,	// -9
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_economic_score_flag);
	enoughScoreButton.create( 214, offsetY+277, 214+19, offsetY+277+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_total_score_flag);
	timeLimitButton.create( 214, offsetY+309, 214+19, offsetY+309+19,		// +23
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_year_limit_flag);

	peopleInc.create( 529, offsetY+206, 
		529+COLOR_OPTION_X_SPACE-1, offsetY+206+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	peopleDec.create( 564, offsetY+206, 
		564+COLOR_OPTION_X_SPACE-1, offsetY+206+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	incomeInc.create( 529, offsetY+238,
		529+COLOR_OPTION_X_SPACE-1, offsetY+238+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	incomeDec.create( 564, offsetY+238,
		564+COLOR_OPTION_X_SPACE-1, offsetY+238+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	scoreInc.create( 529, offsetY+270,
		529+COLOR_OPTION_X_SPACE-1, offsetY+270+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	scoreDec.create( 564, offsetY+270,
		564+COLOR_OPTION_X_SPACE-1, offsetY+270+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	yearInc.create( 529, offsetY+302,
		529+COLOR_OPTION_X_SPACE-1, offsetY+302+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	yearDec.create( 564, offsetY+302,
		564+COLOR_OPTION_X_SPACE-1, offsetY+302+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );

	Button3D startButton, returnButton;
	// startButton.create(250, 553, "BB-START", "BB-START", 1, 0);
	// returnButton.create(440, 553, "BB-RETURN", "BB-RETURN", 1, 0);
	startButton.create(194, 538, "START-U", "START-D", 1, 0);
	returnButton.create(440, 538, "CANCEL-U", "CANCEL-D", 1, 0);

	vga_front.unlock_buf();

	while(1)
	{
		if( sys.need_redraw_flag )
		{
			refreshFlag = SGOPTION_ALL;
			sys.need_redraw_flag = 0;
		}

		vga_front.lock_buf();

		sys.yield();
		mouse.get_event();

		if( sys.signal_exit_flag == 1 )
		{
				retFlag = 0;
				break;
		}

		// -------- display ----------//
		if( refreshFlag )
		{
			// ------- display basic option ---------//
			if( optionMode == OPTION_BASIC )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "SPG-BSC");
#if(MAX_RACE == 10)
					// protection : image_menu.put_to_buf( &vga_back, "SPG-BSC");
					// ensure the user has the release version (I_MENU.RES)
					// image_menu2.put_to_buf( &vga_back, "SPG-BSC") get the real one
					image_menu2.put_to_buf( &vga_back, "SPG-BSC");
#endif
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_RACE )
					raceGroup.paint( reverse_race_table[tempConfig.race_id-1] );
				if( refreshFlag & SGOPTION_COLOR )
					colorGroup.paint( tempConfig.player_nation_color-1 );
				if( refreshFlag & SGOPTION_AI_NATION )
				{
					#if(Y_SHIFT_FLAG)
						#define Y_SHIFT 14
					#else
						#define Y_SHIFT 0
					#endif
					font_san.center_put(564, offsetY+154+Y_SHIFT, 564+25, offsetY+154+Y_SHIFT+21,
						misc.format(tempConfig.ai_nation_count), 1);
					aiNationInc.paint();
					aiNationDec.paint();
					#undef Y_SHIFT
				}
				if( refreshFlag & SGOPTION_DIFFICULTY )
					diffGroup.paint(tempConfig.difficulty_level);
				if( refreshFlag & SGOPTION_TERRAIN )
					terrainGroup.paint(tempConfig.terrain_set-1);
				if( refreshFlag & SGOPTION_LAND_MASS )
					landGroup.paint(tempConfig.land_mass-1);
				if( refreshFlag & SGOPTION_NAME_FIELD )
					playerNameField.paint();
			}

			// ------- display advanced option ---------//
			if( optionMode == OPTION_ADVANCED )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "SPG-O1");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_MAP_ID )
					mapIdField.paint();
				if( refreshFlag & SGOPTION_EXPLORED )
					exploreGroup.paint(1-tempConfig.explore_whole_map);
				if( refreshFlag & SGOPTION_FOG )
					fogGroup.paint(1-tempConfig.fog_of_war);
				if( refreshFlag & SGOPTION_TREASURE )
					treasureGroup.paint( tempConfig.start_up_cash-1 );
				if( refreshFlag & SGOPTION_AI_TREASURE )
					aiTreasureGroup.paint( tempConfig.ai_start_up_cash-1 );
				if( refreshFlag & SGOPTION_AI_AGGRESSIVE )
					aiAggressiveGroup.paint(tempConfig.ai_aggressiveness-1);
				if( refreshFlag & SGOPTION_FRYHTANS )
					monsterGroup.paint(tempConfig.monster_type);
				if( refreshFlag & SGOPTION_RANDOM_STARTUP )
					randomStartUpGroup.paint(1-tempConfig.random_start_up);
			}

			// ------- display advanced option ---------//
			if( optionMode == OPTION_ADVANCE2 )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "SPG-O2");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_RAW )
				{
					font_san.center_put(337, offsetY+105, 337+25, offsetY+105+21,
						misc.format(tempConfig.start_up_raw_site), 1);
					rawSiteInc.paint();
					rawSiteDec.paint();
				}
				if( refreshFlag & SGOPTION_NEAR_RAW )
					nearRawGroup.paint(1-tempConfig.start_up_has_mine_nearby);
				if( refreshFlag & SGOPTION_START_TOWN )
					townStartGroup.paint(
					tempConfig.start_up_independent_town >= 30 ? 2 :
					tempConfig.start_up_independent_town <= 7 ? 0 :
					1
					);
				if( refreshFlag & SGOPTION_TOWN_STRENGTH )
					townResistGroup.paint(tempConfig.independent_town_resistance-1);
				if( refreshFlag & SGOPTION_TOWN_EMERGE )
					townEmergeGroup.paint(1-tempConfig.new_independent_town_emerge);
				if( refreshFlag & SGOPTION_KINGDOM_EMERGE )
					nationEmergeGroup.paint(1-tempConfig.new_nation_emerge);
				if( refreshFlag & SGOPTION_RANDOM_EVENT )
					randomEventGroup.paint(tempConfig.random_event_frequency);
			}

			// ------- display goal option ---------//
			if( optionMode == OPTION_GOAL )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "SPG-GOAL");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_CLEAR_ENEMY )
					clearEnemyButton.paint();
				if( refreshFlag & SGOPTION_CLEAR_MONSTER )
					clearMonsterButton.paint(tempConfig.goal_destroy_monster);
				if( refreshFlag & SGOPTION_ENOUGH_PEOPLE )
				{
					enoughPeopleButton.paint(tempConfig.goal_population_flag);
					font_san.center_put( 456, offsetY+211, 456+67, offsetY+211+21,
						misc.format(tempConfig.goal_population) ,1);
					peopleInc.paint();
					peopleDec.paint();
				}
				if( refreshFlag & SGOPTION_ENOUGH_INCOME )
				{
					enoughIncomeButton.paint(tempConfig.goal_economic_score_flag);
					font_san.center_put( 456, offsetY+243, 456+67, offsetY+243+21,
						misc.format(tempConfig.goal_economic_score), 1);
					incomeInc.paint();
					incomeDec.paint();
				}
				if( refreshFlag & SGOPTION_ENOUGH_SCORE )
				{
					enoughScoreButton.paint(tempConfig.goal_total_score_flag);
					font_san.center_put( 456, offsetY+275, 456+67, offsetY+275+21,
						misc.format(tempConfig.goal_total_score), 1);
					scoreInc.paint();
					scoreDec.paint();
				}
				if( refreshFlag & SGOPTION_TIME_LIMIT )
				{
					timeLimitButton.paint(tempConfig.goal_year_limit_flag);
					font_san.center_put( 456, offsetY+307, 456+33, offsetY+307+21,
						misc.format(tempConfig.goal_year_limit), 1);
					yearInc.paint();
					yearDec.paint();
				}
			}

			// ------- display difficulty -------//
			if( refreshFlag & SGOPTION_DIFFICULTY )
			{
				font_san.center_put( 718, offsetY+74, 780, offsetY+108,
					misc.format(tempConfig.single_player_difficulty()), 1 );
			}

			// -------- repaint button -------//
			if( refreshFlag & SGOPTION_PAGE )
			{
				startButton.paint();
				returnButton.paint();
			}

			refreshFlag = 0;
		}
		sys.blt_virtual_buf();

		if( config.music_flag )
		{
			if( !music.is_playing(1) )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();

		// --------- detect basic option -------- //
		if( optionMode == OPTION_BASIC )
		{
			if( raceGroup.detect() >= 0)
			{
				tempConfig.race_id = raceGroup[raceGroup()].custom_para.value;
				//refreshFlag |= SGOPTION_RACE;
			}
			else if( colorGroup.detect() >= 0)
			{
				tempConfig.player_nation_color = colorGroup[colorGroup()].custom_para.value;
				//refreshFlag |= SGOPTION_COLOR;
			}
			else if( aiNationInc.detect() )
			{
				tempConfig.ai_nation_count++;
				if( tempConfig.ai_nation_count >= MAX_NATION )
					tempConfig.ai_nation_count = MAX_NATION-1;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				refreshFlag |= SGOPTION_AI_NATION | SGOPTION_DIFFICULTY;
			}
			else if( aiNationDec.detect() )
			{
				tempConfig.ai_nation_count--;
				if( tempConfig.ai_nation_count <= 0 )
					tempConfig.ai_nation_count = 1;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				refreshFlag |= SGOPTION_AI_NATION | SGOPTION_DIFFICULTY;
			}
			else if( diffGroup.detect() >= 0)
			{
				if( diffGroup[diffGroup()].custom_para.value != OPTION_CUSTOM )
				{
					tempConfig.change_difficulty(diffGroup[diffGroup()].custom_para.value);
					// all but SGOPTION_PAGE;
					refreshFlag |= SGOPTION_ALL & ~SGOPTION_PAGE;
				}
			}
			else if( terrainGroup.detect() >= 0)
			{
				tempConfig.terrain_set = terrainGroup[terrainGroup()].custom_para.value;
				static short latitudeArray[3] = { 45, 70, 20 };
				err_when( tempConfig.terrain_set <= 0 || tempConfig.terrain_set > 3 );
				tempConfig.latitude = latitudeArray[tempConfig.terrain_set-1];
				//refreshFlag |= SGOPTION_TERRAIN;
			}
			else if( landGroup.detect() >= 0)
			{
				tempConfig.land_mass = landGroup[landGroup()].custom_para.value;
				//refreshFlag |= SGOPTION_LAND_MASS;
			}
			else if( playerNameField.detect() )
			{
				refreshFlag |= SGOPTION_NAME_FIELD;
			}
		}

		// -------- detect advanced option ---------//

		if( optionMode == OPTION_ADVANCED )
		{
			if( mapIdField.detect() )
			{
				refreshFlag |= SGOPTION_MAP_ID;
			}
			else
			if( exploreGroup.detect() >= 0 )
			{
				tempConfig.explore_whole_map = exploreGroup[exploreGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_EXPLORED;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( fogGroup.detect() >= 0 )
			{
				tempConfig.fog_of_war = fogGroup[fogGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_FOG
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( treasureGroup.detect() >= 0 )
			{
				tempConfig.start_up_cash = treasureGroup[treasureGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_TREASURE;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( aiTreasureGroup.detect() >= 0 )
			{
				tempConfig.ai_start_up_cash = aiTreasureGroup[aiTreasureGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_AI_TREASURE;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( aiAggressiveGroup.detect() >= 0 )
			{
				tempConfig.ai_aggressiveness = 
					aiAggressiveGroup[aiAggressiveGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_AI_AGGRESSIVE;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( monsterGroup.detect() >= 0 )
			{
				tempConfig.monster_type = monsterGroup[monsterGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_FRYHTANS;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( randomStartUpGroup.detect() >= 0)
			{
				tempConfig.random_start_up = randomStartUpGroup[randomStartUpGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_RANDOM_STARTUP;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
		}

		// -------- detect advanced option ---------//

		if( optionMode == OPTION_ADVANCE2 )
		{
			if( rawSiteInc.detect() )
			{
				if( ++tempConfig.start_up_raw_site > 7 )
					tempConfig.start_up_raw_site = 7;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				refreshFlag |= SGOPTION_RAW | SGOPTION_DIFFICULTY;
			}
			else if( rawSiteDec.detect() )
			{
				if( --tempConfig.start_up_raw_site < 1 )
					tempConfig.start_up_raw_site = 1;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				refreshFlag |= SGOPTION_RAW | SGOPTION_DIFFICULTY;

			}
			else if( nearRawGroup.detect() >= 0)
			{
				tempConfig.start_up_has_mine_nearby = nearRawGroup[nearRawGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_NEAR_RAW;
				refreshFlag |= SGOPTION_DIFFICULTY;

			}
			else if( townStartGroup.detect() >= 0)
			{
				tempConfig.start_up_independent_town = townStartGroup[townStartGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// resfreshFlag |= SGOPTION_START_TOWN;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( townResistGroup.detect() >= 0)
			{
				tempConfig.independent_town_resistance = townResistGroup[townResistGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// resfreshFlag |= SGOPTION_TOWN_RESIST;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( townEmergeGroup.detect() >= 0)
			{
				tempConfig.new_independent_town_emerge = townEmergeGroup[townEmergeGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_TOWN_EMERGE;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( nationEmergeGroup.detect() >= 0)
			{
				tempConfig.new_nation_emerge = nationEmergeGroup[nationEmergeGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_NATION_EMERGE;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
			else if( randomEventGroup.detect() >= 0)
			{
				tempConfig.random_event_frequency = randomEventGroup[randomEventGroup()].custom_para.value;
				tempConfig.difficulty_level = OPTION_CUSTOM;
				// refreshFlag |= SGOPTION_RANDOM_EVENT;
				refreshFlag |= SGOPTION_DIFFICULTY;
			}
		}

		// -------- detect goal option ----------//

		if( optionMode == OPTION_GOAL )
		{
			if( clearEnemyButton.detect() )
			{
			}
			else if( clearMonsterButton.detect() )
			{
				tempConfig.goal_destroy_monster = clearMonsterButton.pushed_flag;
			}
			else if( enoughPeopleButton.detect() )
			{
				tempConfig.goal_population_flag = enoughPeopleButton.pushed_flag;
			}
			else if( enoughIncomeButton.detect() )
			{
				tempConfig.goal_economic_score_flag = enoughIncomeButton.pushed_flag;
			}
			else if( enoughScoreButton.detect() )
			{
				tempConfig.goal_total_score_flag = enoughScoreButton.pushed_flag;
			}
			else if( timeLimitButton.detect() )
			{
				tempConfig.goal_year_limit_flag = timeLimitButton.pushed_flag;
			}
			else if( peopleInc.detect() )
			{
				tempConfig.goal_population += 100;
				if( tempConfig.goal_population > 5000 )
					tempConfig.goal_population = 5000;
				refreshFlag |= SGOPTION_ENOUGH_PEOPLE;
			}
			else if( peopleDec.detect() )
			{
				tempConfig.goal_population -= 100;
				if( tempConfig.goal_population < 100 )
					tempConfig.goal_population = 100;
				refreshFlag |= SGOPTION_ENOUGH_PEOPLE;
			}
			else if( incomeInc.detect() )
			{
				tempConfig.goal_economic_score += 100;
				if( tempConfig.goal_economic_score > 5000 )
				{
					tempConfig.goal_economic_score = 5000;
				}
				refreshFlag |= SGOPTION_ENOUGH_INCOME;
			}
			else if( incomeDec.detect() )
			{
				tempConfig.goal_economic_score -= 100;
				if( tempConfig.goal_economic_score < 100 )
				{
					tempConfig.goal_economic_score = 100;
				}
				refreshFlag |= SGOPTION_ENOUGH_INCOME;
			}
			else if( scoreInc.detect() )
			{
				if( tempConfig.goal_total_score >= 2000 )
					tempConfig.goal_total_score += 500;
				else
					tempConfig.goal_total_score += 100;
				if( tempConfig.goal_total_score > 10000 )
					tempConfig.goal_total_score = 10000;
				refreshFlag |= SGOPTION_ENOUGH_SCORE;
			}
			else if( scoreDec.detect() )
			{
				if( tempConfig.goal_total_score > 2000 )
					tempConfig.goal_total_score -= 500;
				else
					tempConfig.goal_total_score -= 100;
				if( tempConfig.goal_total_score < 100 )
					tempConfig.goal_total_score = 100;
				refreshFlag |= SGOPTION_ENOUGH_SCORE;
			}
			else if( yearInc.detect() )
			{
				if( tempConfig.goal_year_limit >= 20 )
					tempConfig.goal_year_limit += 5;
				else
					tempConfig.goal_year_limit++;
				if( tempConfig.goal_year_limit > 100 )
				{
					tempConfig.goal_year_limit = 100;
				}
				refreshFlag |= SGOPTION_TIME_LIMIT;
			}
			else if( yearDec.detect() )
			{
				if( tempConfig.goal_year_limit > 20 )
					tempConfig.goal_year_limit -= 5;
				else
					tempConfig.goal_year_limit--;
				if( tempConfig.goal_year_limit < 1 )
				{
					tempConfig.goal_year_limit = 1;
				}
				refreshFlag |= SGOPTION_TIME_LIMIT;
			}
		}

		// --------- detect switch option button ------//

		if( mouse.single_click(96, offsetY+12, 218, offsetY+54) )
		{
			if( optionMode != OPTION_BASIC )
			{
				optionMode = OPTION_BASIC;
				refreshFlag = SGOPTION_ALL;
			}
		}
		else if( mouse.single_click(236, offsetY+12, 363, offsetY+54) )
		{
			if( optionMode != OPTION_ADVANCED )
			{
				optionMode = OPTION_ADVANCED;
				refreshFlag = SGOPTION_ALL;
			}
		}
		else if( mouse.single_click(380, offsetY+12, 506, offsetY+54) )
		{
			if( optionMode != OPTION_ADVANCE2 )
			{
				optionMode = OPTION_ADVANCE2;
				refreshFlag = SGOPTION_ALL;
			}
		}
		else if( mouse.single_click(523, offsetY+12, 649, offsetY+54) )
		{
			if( optionMode != OPTION_GOAL )
			{
				optionMode = OPTION_GOAL;
				refreshFlag = SGOPTION_ALL;
			}
		}

		// --------- detect input name --------//

		// --------- detect start button --------//
		if( startButton.detect() )
		{
			retFlag = 1;
			break;			// break while(1)
		}
		else if( returnButton.detect() )
		{
			retFlag = 0;
			break;			// break while(1)
		}

		vga_front.unlock_buf();

	}

	if( !vga_front.buf_locked )
		vga_front.lock_buf();

	if( retFlag )
	{
		info.init_random_seed( atol(mapIdStr) );
		tempConfig.difficulty_rating = tempConfig.single_player_difficulty();

		config = tempConfig;
	}

	return retFlag;
}
#ifdef Y_SHIFT
	#error
#endif
#undef Y_SHIFT_FLAG

static void disp_virtual_button(ButtonCustom *button, int)
{
	mouse.hide_area(button->x1, button->y1, button->x2, button->y2);
	if( !button->pushed_flag )
	{
		// copy from back buffer to front buffer
		IMGcopy(vga_front.buf_ptr(), vga_front.buf_pitch(),
			vga_back.buf_ptr(), vga_back.buf_pitch(),
			button->x1, button->y1, button->x2, button->y2 );
	}
	else
	{
		// copy from back buffer to front buffer, but the area is
		// darkened by 2 scale
		IMGcopyRemap(vga_front.buf_ptr(), vga_front.buf_pitch(),
			vga_back.buf_ptr(), vga_back.buf_pitch(),
			button->x1, button->y1, button->x2, button->y2,
			vga.vga_color_table->get_table(-2) );

		// draw black frame
		if( button->x2-button->x1+1 == BASIC_OPTION_X_SPACE &&
			button->y2-button->y1+1 == BASIC_OPTION_HEIGHT )
		{
			image_interface.put_front(button->x1, button->y1, "BAS_DOWN");
		}
		else if( button->x2-button->x1+1 == COLOR_OPTION_X_SPACE &&
			button->y2-button->y1+1 == COLOR_OPTION_HEIGHT )
		{
			image_interface.put_front(button->x1, button->y1, "COL_DOWN");
		}
	}
	mouse.show_area();
}


static void disp_virtual_tick(ButtonCustom *button, int )
{
	mouse.hide_area(button->x1, button->y1, button->x2, button->y2);

	// copy from back buffer to front buffer
	IMGcopy(vga_front.buf_ptr(), vga_front.buf_pitch(),
		vga_back.buf_ptr(), vga_back.buf_pitch(),
		button->x1, button->y1, button->x2, button->y2 );

	if( button->pushed_flag )
		image_menu.put_front( button->x1+3, button->y1+3, "NMPG-RCH" );

	mouse.show_area();
}
