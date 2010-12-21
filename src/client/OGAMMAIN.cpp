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

//Filename    : OGAMMAIN.CPP
//Description : Main Game Object - Main menu

#include <OVGA.h>
#include <vga_util.h>
#include <OIMGRES.h>
#include <OSYS.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OFONT.h>
#include <OTUTOR.h>
#include <OBUTTON.h>
#include <OBATTLE.h>
#include <OGFILE.h>
#include <OMUSIC.h>
#include <OGAME.h>
#include <OVGALOCK.h>

#ifdef DEMO
#define DISABLE_MULTI_PLAYER
#define DISABLE_SINGLE_PLAYER_NEW_GAME
#endif

//----------- Define structure ---------//

struct OptionInfo
{
	short x1, y1, x2, y2;
};

// -------- define constant ----------//

enum { SWORD1_X = 258, SWORD1_Y = 194 };

//---------- Begin of function Game::main_menu ----------//
//
void Game::main_menu()
{
	enum { MAIN_OPTION_COUNT = 6 };

	static OptionInfo main_option_array[MAIN_OPTION_COUNT] =
	{
		{ 6+SWORD1_X,  11+SWORD1_Y, 282+SWORD1_X ,  63+SWORD1_Y },
		{ 6+SWORD1_X,  68+SWORD1_Y, 282+SWORD1_X , 113+SWORD1_Y },
		{ 6+SWORD1_X, 121+SWORD1_Y, 282+SWORD1_X , 176+SWORD1_Y },
		// ####### begin Gilbert 20/10 ######//
		// { 6+SWORD1_X, 189+SWORD1_Y, 282+SWORD1_X , 219+SWORD1_Y },
		{ 6+SWORD1_X, 184+SWORD1_Y, 282+SWORD1_X , 222+SWORD1_Y },
		// ####### end Gilbert 20/10 ######//
		{ 6+SWORD1_X, 224+SWORD1_Y, 282+SWORD1_X , 295+SWORD1_Y },
		{40+SWORD1_X, 297+SWORD1_Y, 254+SWORD1_X , 337+SWORD1_Y },
	};

	char main_option_flag[MAIN_OPTION_COUNT] = 
	{
		1, 1, 1, 1, 1, 1,
	};

	// test sys.game_version, skip single player
	main_option_flag[0] = (sys.game_version == VERSION_MULTIPLAYER_ONLY) ? 0 : 1;
//	main_option_flag[0] = 1;

	// ###### begin Gilbert 25/9 #######//
#ifdef DISABLE_MULTI_PLAYER
	// disable multiplayer game, Game::multi_player_menu is disabled
	main_option_flag[1] = 0;
#endif
	// ###### end Gilbert 25/9 #######//

	// skip encyclopedia if not found
	main_option_flag[2] = *sys.dir_encyc ? 1 : 0;

	//------------- display menu options -------------//

	int refreshFlag=1, i;

	mouse_cursor.set_icon(CURSOR_NORMAL);
	vga_front.bar(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,V_BLACK);
	OptionInfo* optionInfo;
	char *menuBitmap = NULL;
	char *brightBitmap = NULL;
	char *darkBitmap = NULL;
	int pointingOption = -1;

	while(1)
	{
		game_mode = GAME_PREGAME;

		//------- Display game title and paint menu box --------//

		if( refreshFlag )
		{
			mouse_cursor.set_icon(CURSOR_NORMAL);

			image_interface.put_to_buf( &vga_back, "M_MAIN" );

			vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1);	// blt the main menu screen from the back buffer to the front buffer

			disp_version();

			if(!menuBitmap)
			{
				int resSize;
				File *resFile;
				resFile = image_interface.get_file("SWRD-1", resSize);
				menuBitmap = mem_add(resSize);
				resFile->file_read(menuBitmap, resSize);
			}
			if(!brightBitmap)
			{
				int resSize;
				File *resFile;
				resFile = image_interface.get_file("SWRD-1B", resSize);
				brightBitmap = mem_add(resSize);
				resFile->file_read(brightBitmap, resSize);
			}
			if(!darkBitmap)
			{
				int resSize;
				File *resFile;
				resFile = image_interface.get_file("SWRD-1C", resSize);
				darkBitmap = mem_add(resSize);
				resFile->file_read(darkBitmap, resSize);
			}
			
			for( i = 0; i < MAIN_OPTION_COUNT; ++i )
			{
				if( main_option_flag[i] >= 0 )
				{
					mouse.hide_area(main_option_array[i].x1, main_option_array[i].y1,
						main_option_array[i].x2, main_option_array[i].y2);
					vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, 
						main_option_flag[i] ? menuBitmap : darkBitmap,
						main_option_array[i].x1 - SWORD1_X, main_option_array[i].y1 - SWORD1_Y,
						main_option_array[i].x2 - SWORD1_X, main_option_array[i].y2 - SWORD1_Y);
					mouse.show_area();
				}
			}
			pointingOption = -1;
			refreshFlag=0;
		}

		if( config.music_flag )
		{
			if( !music.is_playing(1) )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
		{
			music.stop();
		}

		// display main menu
		int newPointingOption = -1;
		for(i = 0, optionInfo = main_option_array; i < MAIN_OPTION_COUNT; ++i, ++optionInfo)
		{
			if( main_option_flag[i] > 0 &&
				mouse.in_area(optionInfo->x1, optionInfo->y1, optionInfo->x2, optionInfo->y2) )
			{
				newPointingOption = i;
				break;
			}
		}

		if( pointingOption != newPointingOption)
		{
			err_when( !menuBitmap );
			err_when( !brightBitmap );
			err_when( !darkBitmap );

			// put un-highlighted option back
			i = pointingOption;
			if( i >= 0 && i < MAIN_OPTION_COUNT )
			{
				mouse.hide_area(main_option_array[i].x1, main_option_array[i].y1,
					main_option_array[i].x2, main_option_array[i].y2);
				vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, menuBitmap,
					main_option_array[i].x1 - SWORD1_X, main_option_array[i].y1 - SWORD1_Y,
					main_option_array[i].x2 - SWORD1_X, main_option_array[i].y2 - SWORD1_Y);
				mouse.show_area();
			}

			// put new hightlighted option
			i = newPointingOption;
			if( i >= 0 && i < MAIN_OPTION_COUNT )
			{
				mouse.hide_area(main_option_array[i].x1, main_option_array[i].y1,
					main_option_array[i].x2, main_option_array[i].y2);
				vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, brightBitmap,
					main_option_array[i].x1 - SWORD1_X, main_option_array[i].y1 - SWORD1_Y,
					main_option_array[i].x2 - SWORD1_X, main_option_array[i].y2 - SWORD1_Y);
				mouse.show_area();
			}
			pointingOption = newPointingOption;
		}
		// ######### end Gilbert 23/7 ##########//

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		//---------- detect buttons -----------//

		sys.yield();
		mouse.get_event();

		optionInfo = main_option_array;

		sys.signal_exit_flag = 0;

		for( i=0 ; i<MAIN_OPTION_COUNT ; i++, optionInfo++ )
		{
			if( main_option_flag[i] > 0 && 
				mouse.single_click( optionInfo->x1, optionInfo->y1, optionInfo->x2, optionInfo->y2 ) )
			{
				// ------- free some resource for a moment --------//
				if( menuBitmap )
				{
					mem_del(menuBitmap);
					menuBitmap = NULL;
				}
				if( brightBitmap)
				{
					mem_del(brightBitmap);
					brightBitmap = NULL;
				}
				if( darkBitmap)
				{
					mem_del(darkBitmap);
					darkBitmap = NULL;
				}

				run_main_menu_option(i+1);
				refreshFlag=1;
				break;
			}
		}

		//-------------------------------------//

		if( sys.signal_exit_flag == 1 || i == MAIN_OPTION_COUNT-1 )			// quit the system now
			break;
	}

	//--------------------------------------//

	music.stop();

	if( menuBitmap )
		mem_del(menuBitmap);

	if( brightBitmap)
		mem_del(brightBitmap);

	if( darkBitmap)
		mem_del(darkBitmap);

	//--------------------------------------//

}
//------------ End of function Game::main_menu -----------//


//-------- Begin of function Game::run_main_menu_option --------//
//
// Run the selected game option.
//
void Game::run_main_menu_option(int optionId)
{
	//------- Single Player Game -------//

	if( optionId==1 )
	{
		single_player_menu();
	}

	//-------- Multiplayer Game ----------//

	if( optionId==2 )
	{
#ifndef DISABLE_MULTI_PLAYER
		game_mode = GAME_MULTI_PLAYER;
		multi_player_menu(0, NULL);
		// multi_player_game();
#endif
	}

	//----------- Encyclopedia -----------//

	if( optionId==3 )
	{
		game_mode = GAME_ENCYCLOPEDIA;
		view_encyclopedia();
	}

	//----------- Hall of Fame -----------//

	if( optionId==4 )
	{
		game_file_array.disp_hall_of_fame();
	}

	//------------- Credits -----------//

	// ####### begin Gilbert 2/9 #######//
	if( optionId==5 )
	{
		if( m.is_file_exist("TESTING2.SYS") )
		{
			game_mode = GAME_TEST;          // testing game instead
			test_game();
		}
		else
		{
			game_mode = GAME_CREDITS;
			view_credits();
		}
	}
	// ####### end Gilbert 2/9 #######//

	// ####### begin Gilbert 7/11 #########//
	if( optionId==6 )
	{
		sys.signal_exit_flag = 1;
	}
	// ####### end Gilbert 7/11 #########//
}
//---------- End of function Game::run_main_menu_option ---------//


//-------- Begin of static function disp_version --------//
//
void Game::disp_version()
{
	//----------- display version string --------//

	String str;

	str  = "Version ";
	str += GAME_VERSION_STR;

	#ifdef DEMO
		str = "Demo Version";
	#endif

	#ifdef BETA
		str = "This is a Beta version. Unauthorized distribution of this Beta is illegal.";
	#endif

	if( str.len() > 40 )
		font_news.center_put( 0, VGA_HEIGHT-20, VGA_WIDTH-1, VGA_HEIGHT-1, str );
	else
		font_news.right_put( VGA_WIDTH-10, VGA_HEIGHT-20, str );
}
//---------- End of function Game::disp_version ---------//


//---------- Begin of function Game::single_player_menu ----------//
//
void Game::single_player_menu()
{
	enum { SINGLE_PLAYER_OPTION_COUNT = 5 };

#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME 
	static OptionInfo single_player_option_array[SINGLE_PLAYER_OPTION_COUNT] =
	{
		{ 5+SWORD1_X,  10+SWORD1_Y, 282+SWORD1_X,  62+SWORD1_Y },
		{ 5+SWORD1_X,  67+SWORD1_Y, 282+SWORD1_X, 112+SWORD1_Y },
		{ 5+SWORD1_X, 120+SWORD1_Y, 282+SWORD1_X, 175+SWORD1_Y },
		{ 5+SWORD1_X, 182+SWORD1_Y, 282+SWORD1_X, 223+SWORD1_Y },
		{40+SWORD1_X, 238+SWORD1_Y, 254+SWORD1_X, 280+SWORD1_Y },
	};

	static char single_player_option_flag[SINGLE_PLAYER_OPTION_COUNT] =
	{
		1, 1, 1, 1, 1,				// 1 = in use, 0 = darken, -1 = invisible
	};
#else
	static OptionInfo single_player_option_array[SINGLE_PLAYER_OPTION_COUNT] =
	{
		{ 2+SWORD1_X,  10+SWORD1_Y, 286+SWORD1_X,  65+SWORD1_Y },
		{ 2+SWORD1_X,  67+SWORD1_Y, 286+SWORD1_X, 109+SWORD1_Y },
		{ 2+SWORD1_X, 112+SWORD1_Y, 286+SWORD1_X, 171+SWORD1_Y },
		{ 2+SWORD1_X, 112+SWORD1_Y, 286+SWORD1_X, 171+SWORD1_Y },       // not used
		{38+SWORD1_X, 174+SWORD1_Y, 256+SWORD1_X, 216+SWORD1_Y },
	};

	static char single_player_option_flag[SINGLE_PLAYER_OPTION_COUNT] =
	{
		1, 1, 1, -1, 1,                                // 1 = in use, 0 = darken, -1 = invisible
	};
#endif

	game_mode = GAME_SINGLE_PLAYER;

	//------ display the single player menu options ------//

	// image_interface.put_large( &vga_back, 247, 242, "M_SINGLE" );
	// vga_util.blt_buf(247, 242, 545, 567);
	// sys.blt_virtual_buf();		// blt the virtual front buffer to the screen
	int refreshFlag = 1, i;
	mouse_cursor.set_icon(CURSOR_NORMAL);
	char *menuBitmap = NULL;
	char *brightBitmap = NULL;
	char *darkBitmap = NULL;
	int pointingOption = -1;


	//---------- detect buttons -----------//

	while(1)
	{
		sys.yield();
		mouse.get_event();

		if( refreshFlag )
		{
			image_interface.put_to_buf( &vga_back, "M_MAIN" );

			vga_util.blt_buf(0,0,VGA_WIDTH-1, VGA_HEIGHT-1);

			if(!menuBitmap)
			{
				int resSize;
				File *resFile;
#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME 
				resFile = image_interface.get_file("SWRD-2", resSize);
#else
				resFile = image_interface.get_file("SWRD-4", resSize);
#endif
				menuBitmap = mem_add(resSize);
				resFile->file_read(menuBitmap, resSize);
			}
			if(!brightBitmap)
			{
				int resSize;
				File *resFile;
#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME
				resFile = image_interface.get_file("SWRD-2B", resSize);
#else
				resFile = image_interface.get_file("SWRD-4B", resSize);
#endif
				brightBitmap = mem_add(resSize);
				resFile->file_read(brightBitmap, resSize);
			}
			if(!darkBitmap)
			{
				int resSize;
				File *resFile;
#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME
				resFile = image_interface.get_file("SWRD-2C", resSize);
#else
				resFile = image_interface.get_file("SWRD-2C", resSize); // no SWRD-4C
#endif
				darkBitmap = mem_add(resSize);
				resFile->file_read(darkBitmap, resSize);
			}

			for( i = 0; i < SINGLE_PLAYER_OPTION_COUNT; ++i )
			{
				if( single_player_option_flag[i] >= 0)
				{
					mouse.hide_area(single_player_option_array[i].x1, single_player_option_array[i].y1,
						single_player_option_array[i].x2, single_player_option_array[i].y2);
					vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, 
						single_player_option_flag[i] ? menuBitmap : darkBitmap,
						single_player_option_array[i].x1 - SWORD1_X, single_player_option_array[i].y1 - SWORD1_Y,
						single_player_option_array[i].x2 - SWORD1_X, single_player_option_array[i].y2 - SWORD1_Y);
					mouse.show_area();
				}
			}
			pointingOption = -1;
			refreshFlag=0;
		}

		// ###### begin Gilbert 18/9 ########//
		if( config.music_flag )
		{
			if( !music.is_playing(1) )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();
		// ###### end Gilbert 18/9 ########//

		// display main menu
		int newPointingOption = -1;
		for(i = 0; i < SINGLE_PLAYER_OPTION_COUNT; ++i)
		{
			if( single_player_option_flag[i] > 0 &&
				mouse.in_area(single_player_option_array[i].x1, single_player_option_array[i].y1,
				single_player_option_array[i].x2, single_player_option_array[i].y2) )
			{
				newPointingOption = i;
				break;
			}
		}

		if( pointingOption != newPointingOption)
		{
			err_when( !menuBitmap );
			err_when( !brightBitmap );
			err_when( !darkBitmap );

			// put un-highlighted option back
			i = pointingOption;
			if( i >= 0 && i < SINGLE_PLAYER_OPTION_COUNT )
			{
				mouse.hide_area(single_player_option_array[i].x1, single_player_option_array[i].y1,
					single_player_option_array[i].x2, single_player_option_array[i].y2);
				vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, menuBitmap,
					single_player_option_array[i].x1 - SWORD1_X, single_player_option_array[i].y1 - SWORD1_Y,
					single_player_option_array[i].x2 - SWORD1_X, single_player_option_array[i].y2 - SWORD1_Y);
				mouse.show_area();
			}

			// put new hightlighted option
			i = newPointingOption;
			if( i >= 0 && i < SINGLE_PLAYER_OPTION_COUNT )
			{
				mouse.hide_area(single_player_option_array[i].x1,single_player_option_array[i].y1,
					single_player_option_array[i].x2,single_player_option_array[i].y2);
				vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, brightBitmap,
					single_player_option_array[i].x1 -SWORD1_X,single_player_option_array[i].y1 -SWORD1_Y,
					single_player_option_array[i].x2 -SWORD1_X,single_player_option_array[i].y2 -SWORD1_Y);
				mouse.show_area();
			}
			pointingOption = newPointingOption;
		}

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		OptionInfo* optionInfo = single_player_option_array;

		for( int i=0 ; i<SINGLE_PLAYER_OPTION_COUNT ; i++, optionInfo++ )
		{
			if( single_player_option_flag[i] > 0 &&
				mouse.single_click( optionInfo->x1, optionInfo->y1, optionInfo->x2, optionInfo->y2 ) )
			{
				// free some resource
				if( menuBitmap )
				{
					mem_del(menuBitmap);
					menuBitmap = NULL;
				}
				if( brightBitmap )
				{
					mem_del(brightBitmap);
					brightBitmap = NULL;
				}
				if( darkBitmap )
				{
					mem_del(darkBitmap);
					darkBitmap = NULL;
				}

				refreshFlag = 1;

				switch(i+1)
				{
					case 1:
						tutor.select_run_tutor(0);		// select and run tutorial, 0-called from the main menu, not from in-game
						break;

					case 2:
#ifndef DEMO
						single_player_game(0);
#else
						select_run_scenario();
#endif
						break;

					case 3:
						game_file_array.init("*.SAV");

						if( game_file_array.menu(2) == 1)
						{
							battle.run_loaded();
							deinit();
						}
						{
							char signalExitFlagBackup = sys.signal_exit_flag;
							sys.signal_exit_flag = 2;
							game.deinit();   // game.deinit() is needed if game_file_array.menu fails
							sys.signal_exit_flag = signalExitFlagBackup;
						}
						break;

					case 4:	
						select_run_scenario();
						break;
				}

				return;
			}
		}
	}

	if( menuBitmap )
		mem_del(menuBitmap);
	if( brightBitmap )
		mem_del(brightBitmap);
	if( darkBitmap )
		mem_del(darkBitmap);
}
//------------ End of function Game::single_player_menu -----------//


//-------- Begin of function Game::test_game --------//
//
// <int> noAI - if there should be no AI in the game.
//
void Game::test_game()
{
	init();

	battle.run_test();

	deinit();
}
//--------- End of function Game::test_game ---------//


#ifndef DISABLE_MULTI_PLAYER
//---------- Begin of function Game::multi_player_menu ----------//
//
// ####### begin Gilbert 13/2 ########//
void Game::multi_player_menu(int lobbied, char *game_host)
// ####### end Gilbert 13/2 ########//
{
	enum { MULTI_PLAYER_OPTION_COUNT = 5 };

	static OptionInfo multi_player_option_array[MULTI_PLAYER_OPTION_COUNT] =
	{
		{ 5+SWORD1_X,  10+SWORD1_Y, 282+SWORD1_X,  62+SWORD1_Y },
		{ 5+SWORD1_X,  67+SWORD1_Y, 282+SWORD1_X, 112+SWORD1_Y },
		{ 5+SWORD1_X, 120+SWORD1_Y, 282+SWORD1_X, 175+SWORD1_Y },
		{ 5+SWORD1_X, 182+SWORD1_Y, 282+SWORD1_X, 223+SWORD1_Y },
		{40+SWORD1_X, 238+SWORD1_Y, 254+SWORD1_X, 280+SWORD1_Y },
	};

	static char multi_player_option_flag[MULTI_PLAYER_OPTION_COUNT] =
	{
		-1, 1, 1, -1, 1,
	};

	game_mode = GAME_MULTI_PLAYER;

	//------ display the multi player menu options ------//

	int refreshFlag = 1, i;
	mouse_cursor.set_icon(CURSOR_NORMAL);
	char *menuBitmap = NULL;
	char *brightBitmap = NULL;
	char *darkBitmap = NULL;
	int pointingOption = -1;


	//---------- detect buttons -----------//

	while(1)
	{
		sys.yield();
		mouse.get_event();

		if( refreshFlag )
		{
			image_interface.put_to_buf( &vga_back, "M_MAIN" );

			vga_util.blt_buf(0,0,VGA_WIDTH-1, VGA_HEIGHT-1);

			if(!menuBitmap)
			{
				int resSize;
				File *resFile;
				resFile = image_interface.get_file("SWRD-2", resSize);
				menuBitmap = mem_add(resSize);
				resFile->file_read(menuBitmap, resSize);
			}
			if(!brightBitmap)
			{
				int resSize;
				File *resFile;
				resFile = image_interface.get_file("SWRD-2B", resSize);
				brightBitmap = mem_add(resSize);
				resFile->file_read(brightBitmap, resSize);
			}
			if(!darkBitmap)
			{
				int resSize;
				File *resFile;
				resFile = image_interface.get_file("SWRD-2C", resSize);
				darkBitmap = mem_add(resSize);
				resFile->file_read(darkBitmap, resSize);
			}

			for( i = 0; i < MULTI_PLAYER_OPTION_COUNT; ++i )
			{
				if( multi_player_option_flag[i] >= 0 )
				{
					mouse.hide_area(multi_player_option_array[i].x1, multi_player_option_array[i].y1,
						multi_player_option_array[i].x2, multi_player_option_array[i].y2);
					vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, 
						multi_player_option_flag[i] ? menuBitmap : darkBitmap,
						multi_player_option_array[i].x1 - SWORD1_X, multi_player_option_array[i].y1 - SWORD1_Y,
						multi_player_option_array[i].x2 - SWORD1_X, multi_player_option_array[i].y2 - SWORD1_Y);
					mouse.show_area();
				}
			}

			pointingOption = -1;
			refreshFlag=0;
		}

		// ###### begin Gilbert 18/9 ########//
		if( config.music_flag )
		{
			if( !music.is_playing(1) )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();
		// ###### end Gilbert 18/9 ########//

		// display main menu
		int newPointingOption = -1;
		for(i = 0; i < MULTI_PLAYER_OPTION_COUNT; ++i)
		{
			if( multi_player_option_flag[i] > 0 &&
				mouse.in_area(multi_player_option_array[i].x1, multi_player_option_array[i].y1,
				multi_player_option_array[i].x2, multi_player_option_array[i].y2) )
			{
				newPointingOption = i;
				break;
			}
		}

		if( pointingOption != newPointingOption)
		{
			err_when( !menuBitmap );
			err_when( !brightBitmap );
			err_when( !darkBitmap );

			// put un-highlighted option back
			i = pointingOption;
			if( i >= 0 && i < MULTI_PLAYER_OPTION_COUNT )
			{
				mouse.hide_area(multi_player_option_array[i].x1, multi_player_option_array[i].y1,
					multi_player_option_array[i].x2, multi_player_option_array[i].y2);
				vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, menuBitmap,
					multi_player_option_array[i].x1 - SWORD1_X, multi_player_option_array[i].y1 - SWORD1_Y,
					multi_player_option_array[i].x2 - SWORD1_X, multi_player_option_array[i].y2 - SWORD1_Y);
				mouse.show_area();
			}

			// put new hightlighted option
			i = newPointingOption;
			if( i >= 0 && i < MULTI_PLAYER_OPTION_COUNT )
			{
				mouse.hide_area(multi_player_option_array[i].x1, multi_player_option_array[i].y1,
					multi_player_option_array[i].x2, multi_player_option_array[i].y2);
				vga_front.put_bitmap_area(SWORD1_X, SWORD1_Y, brightBitmap,
					multi_player_option_array[i].x1 - SWORD1_X, multi_player_option_array[i].y1 - SWORD1_Y,
					multi_player_option_array[i].x2 - SWORD1_X, multi_player_option_array[i].y2 - SWORD1_Y);
				mouse.show_area();
			}
			pointingOption = newPointingOption;
		}

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		OptionInfo* optionInfo = multi_player_option_array;

		for( int i=0 ; i<MULTI_PLAYER_OPTION_COUNT ; i++, optionInfo++ )
		{
			if( multi_player_option_flag[i] > 0 &&
				mouse.single_click( optionInfo->x1, optionInfo->y1, optionInfo->x2, optionInfo->y2 ) )
			{
				// free some resource
				if( menuBitmap )
				{
					mem_del(menuBitmap);
					menuBitmap = NULL;
				}
				if( brightBitmap )
				{
					mem_del(brightBitmap);
					brightBitmap = NULL;
				}
				if( darkBitmap )
				{
					mem_del(darkBitmap);
					darkBitmap = NULL;
				}
				refreshFlag = 1;

				switch(i+1)
				{
					case 2:
						// ####### begin Gilbert 13/2 #######//
						multi_player_game(lobbied, game_host);
						// ####### end Gilbert 13/2 #######//
						break;

					case 3:
						// ##### begin Gilbert 26/8 ######//
						{
							int loadedRecno = 0;
							game_file_array.init("*.SVM");
							if( game_file_array.menu(2, &loadedRecno) == 1 )
							{
								err_when( !loadedRecno );
								// ####### begin Gilbert 13/2 #######//
								load_mp_game(game_file_array[loadedRecno]->file_name, lobbied, game_host);
								// ####### begin Gilbert 13/2 #######//
							}
							{
								char signalExitFlagBackup = sys.signal_exit_flag;
								sys.signal_exit_flag = 2;
								game.deinit();		// game.deinit() is needed if game_file_array.menu fails
								sys.signal_exit_flag = signalExitFlagBackup;
							}
						// ##### end Gilbert 26/8 ######//
						}
						break;

					case 5:
						break;
				}

				return;
			}
		}
	}

	if( menuBitmap )
		mem_del(menuBitmap);
	if( brightBitmap )
		mem_del(brightBitmap);
	if( darkBitmap )
		mem_del(darkBitmap);
}
//------------ End of function Game::multi_player_menu -----------//
#endif

