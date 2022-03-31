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

#include <version.h>
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
#include <OSaveGameArray.h>
#include <OGAMHALL.h>
#include <OMUSIC.h>
#include <OGAME.h>
#include <OVGALOCK.h>
#include "gettext.h"

#ifdef DEMO
#define DISABLE_MULTI_PLAYER
#define DISABLE_SINGLE_PLAYER_NEW_GAME
#endif
# define SWORD_BUTTON_INSTANCES_SIZE 6

//----------- Define structure ---------//

// struct OptionInfo
// {
// 	short x1, y1, x2, y2;
// };

// -------- define constant ----------//

enum { SWORD1_X = 258, SWORD1_Y = 194 };

enum BITMAP_SWORD
{
	IDLE,
	HOVER,
	ACTIVE
};

const char *BITMAP_SWORD_VAL[3] = {"SWRD-1", "SWRD-1B", "SWRD-1C"};
const char *BITMAP_SWORD2_VAL[3] = {"SWRD-2", "SWRD-2B", "SWRD-2C"};
const char *BITMAP_SWORD4_VAL[3] = {"SWRD-4", "SWRD-4B", "SWRD-2C"};

const char *BITMAP_SWORD_SINGLEPLAYER_VAL[3] = {
#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME
	BITMAP_SWORD2_VAL[0],
	BITMAP_SWORD2_VAL[1],
	BITMAP_SWORD2_VAL[2],
#else
	BITMAP_SWORD4_VAL[0],
	BITMAP_SWORD4_VAL[1],
	BITMAP_SWORD4_VAL[2],
#endif
};

struct SwordButton
		{
			short width, height, variant;
};
struct ButtonLocation
{
	short margin_left, margin_top;
};

enum sword_button_variants
{
	SWORD1 = 0,
	SWORD2 = 1,
	SWORD3 = 2,
	SWORD4 = 3,
	SWORD5 = 4,
	SHORT_SWORD = 5
};

static SwordButton SWORD_BUTTON_INSTANCES[SWORD_BUTTON_INSTANCES_SIZE] = {
		{276, 52, SWORD1},
		{276, 45, SWORD2},
		{276, 55, SWORD3},
		{276, 38, SWORD4},
		{276, 71, SWORD5},
		{214, 42, SHORT_SWORD},
};

/**
 * @brief Get the bitmap from the name
 * 
 * @param bitmap_name 
 * @return char* The pointer to the bitmap
 */
char * Game::get_bitmap_by_name(const char* bitmap_name){
	char *bitmap = NULL;
	int resSize;
	File *resFile;
	resFile = image_interface.get_file(bitmap_name, resSize);
	bitmap = mem_add(resSize);
	resFile->file_read(bitmap, resSize);
	return bitmap;
}

/**
 * @brief Update the button on the screen with the desired bitmap 
 * 
 * @param x Start location on X
 * @param y Statt location on Y
 * @param menu_button The button and the two corners (top-left and right-bottom)
 * @param bitmap The bitmap to show (and the variant, normal, bright, darken)
 */
void Game::update_main_menu_button(int x, int y, OptionInfo menu_button, char *bitmap)
{
	mouse.hide_area(menu_button.x1, menu_button.y1,
									menu_button.x2, menu_button.y2);
	vga_front.put_bitmap_area(x, y,
														bitmap,
														menu_button.x1 - x, menu_button.y1 - y,
														menu_button.x2 - x, menu_button.y2 - y);
	mouse.show_area();
}

/**
 * @brief Calculate the coordinates of the new button based upon X and Y reference plus left/top margins
 * 
 * @param start_x Start reference point for X (from left to right)
 * @param start_y Start reference point for Y (from top to bottom)
 * @param button_variant  Index of sword_button_variants
 * @param button_box_array Indicate the margin left and top for the button (think as offset)
 * @return OptionInfo with the 4 coordinates of the element
 */
OptionInfo generate_button(int start_x, int start_y, int button_variant, ButtonLocation button_box_array) {
	err_when(button_variant > SWORD_BUTTON_INSTANCES_SIZE);
	SwordButton definition_button = SWORD_BUTTON_INSTANCES[button_variant];
	short x1 = button_box_array.margin_left + start_x;
	short y1 = button_box_array.margin_top + start_y;
	short x2 = x1 + definition_button.width;
	short y2 = y1 + definition_button.height;
	OptionInfo instance_button = { x1, y1, x2, y2 };
	return instance_button;
}

/**
 * @brief Get the menu button list object, iterating over the button variants, and the box definition (margin/offset).
 * Needs a starting coordinates (x,y) and it will advance over the Y value (prev + margins)
 * 
 * @param[out] source Where the output list will be placed
 * @param size The ammount of buttons, it should be between the lenght of button_variant and button_box_array
 * @param start_x The X starting point, this is fixed (does not add the prev value, just the margin)
 * @param start_y The Y starting point, this gets the offset of previous button on the list, so this displace the new one.
 * @param button_variant The array of buttons that will be added, like SWORD1, SHORT_SWORD
 * @param button_box_array The margin/offset definiton per button, left ant top respectively.
 * @todo Check for the size of the arrays of the for
 */
void get_main_menu_button_list(OptionInfo *source, int size, int start_x, int start_y, int button_variant[], ButtonLocation button_box_array[]){
	short y1 = start_y;
	for (int i = 0; i < size; i++)
	{
		OptionInfo button = generate_button(start_x, y1, button_variant[i], button_box_array[i]);
		y1 = button.y2;
		source[i] = button;
	}
}

//---------- Begin of function Game::main_menu ----------//
//
void Game::main_menu()
{
	enum { MAIN_OPTION_COUNT = 6 };

	static OptionInfo main_option_array[MAIN_OPTION_COUNT];
	ButtonLocation main_option_box_array[MAIN_OPTION_COUNT] = {{6, 11}, {6, 5}, {6, 8}, {6, 8}, {6, 2}, {40, 2}};
	int main_option_button_variant_array[MAIN_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SWORD5, SHORT_SWORD};
	get_main_menu_button_list(main_option_array, MAIN_OPTION_COUNT, SWORD1_X, SWORD1_Y, main_option_button_variant_array, main_option_box_array);
	// {
	// 	{ 6+SWORD1_X,  11+SWORD1_Y, 282+SWORD1_X ,  63+SWORD1_Y },
	// 	{ 6+SWORD1_X,  68+SWORD1_Y, 282+SWORD1_X , 113+SWORD1_Y },
	// 	{ 6+SWORD1_X, 121+SWORD1_Y, 282+SWORD1_X , 176+SWORD1_Y },
	// 	// ####### begin Gilbert 20/10 ######//
	// 	// { 6+SWORD1_X, 189+SWORD1_Y, 282+SWORD1_X , 219+SWORD1_Y },
	// 	{ 6+SWORD1_X, 184+SWORD1_Y, 282+SWORD1_X , 222+SWORD1_Y },
	// 	// ####### end Gilbert 20/10 ######//
	// 	{ 6+SWORD1_X, 224+SWORD1_Y, 282+SWORD1_X , 295+SWORD1_Y },
	// 	{40+SWORD1_X, 297+SWORD1_Y, 254+SWORD1_X , 337+SWORD1_Y },
	// };

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
				menuBitmap = get_bitmap_by_name(BITMAP_SWORD_VAL[BITMAP_SWORD::IDLE]);
			
			if(!brightBitmap)
				brightBitmap = get_bitmap_by_name(BITMAP_SWORD_VAL[BITMAP_SWORD::HOVER]);
			
			if(!darkBitmap)
				darkBitmap = get_bitmap_by_name(BITMAP_SWORD_VAL[BITMAP_SWORD::ACTIVE]);
			
			
			for( i = 0; i < MAIN_OPTION_COUNT; ++i )
			{
				if (main_option_flag[i] >= 0)
					update_main_menu_button(SWORD1_X, SWORD1_Y, main_option_array[i], main_option_flag[i] ? menuBitmap : darkBitmap);
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
				update_main_menu_button(SWORD1_X, SWORD1_Y, main_option_array[i], menuBitmap);

			// put new hightlighted option
			i = newPointingOption;
			if( i >= 0 && i < MAIN_OPTION_COUNT )
				update_main_menu_button(SWORD1_X, SWORD1_Y, main_option_array[i], brightBitmap);
			
			pointingOption = newPointingOption;
		}
		// ######### end Gilbert 23/7 ##########//

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		//---------- detect buttons -----------//

		sys.yield();
		vga.flip();
		mouse.get_event();

		optionInfo = main_option_array;

		// Reset exit-to-main-menu flag
		if (sys.signal_exit_flag == 2) sys.signal_exit_flag = 0;

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

		// method to start a game replay at this time
		if( mouse.is_key_event() && mouse.scan_code == 'r')
		{
			battle.run_replay();
			refreshFlag=1;
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
		hall_of_fame.disp_hall_of_fame();
	}

	//------------- Credits -----------//

	// ####### begin Gilbert 2/9 #######//
	if( optionId==5 )
	{
		if( misc.is_file_exist("TESTING2.SYS") )
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

	str  = _("Version");
	str += " ";
	str += GAME_VERSION_STR;

	#ifdef DEV_VERSION
		str += "-dev";
	#endif

	#ifndef HAVE_KNOWN_BUILD
		str += "?";
	#endif

	#ifdef DEBUG
		str += " (DEBUG)";
	#endif

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
	static OptionInfo single_player_option_array[SINGLE_PLAYER_OPTION_COUNT];
#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME
	ButtonLocation single_player_option_box_array[SINGLE_PLAYER_OPTION_COUNT] = {{6, 10}, {6, 5}, {6, 8}, {6, 8}, {40, 15}};
	static int single_player_option_button_variant_array[SINGLE_PLAYER_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SHORT_SWORD};
	get_main_menu_button_list(single_player_option_array, SINGLE_PLAYER_OPTION_COUNT, SWORD1_X, SWORD1_Y, single_player_option_button_variant_array, single_player_option_box_array);
	// {
	// 	{ 5+SWORD1_X,  10+SWORD1_Y, 282+SWORD1_X,  62+SWORD1_Y },
	// 	{ 5+SWORD1_X,  67+SWORD1_Y, 282+SWORD1_X, 112+SWORD1_Y },
	// 	{ 5+SWORD1_X, 120+SWORD1_Y, 282+SWORD1_X, 175+SWORD1_Y },
	// 	{ 5+SWORD1_X, 182+SWORD1_Y, 282+SWORD1_X, 223+SWORD1_Y },
	// 	{40+SWORD1_X, 238+SWORD1_Y, 254+SWORD1_X, 280+SWORD1_Y },
	// };

	static char single_player_option_flag[SINGLE_PLAYER_OPTION_COUNT] =
	{
		1, 1, 1, 1, 1,				// 1 = in use, 0 = darken, -1 = invisible
	};
#else
	ButtonLocation single_player_option_box_array[SINGLE_PLAYER_OPTION_COUNT] = {{2, 10}, {2, 5}, {2, 3}, {2, -55}, {38, 4}};
	static int single_player_option_button_variant_array[SINGLE_PLAYER_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SHORT_SWORD};
	get_main_menu_button_list(single_player_option_array, SINGLE_PLAYER_OPTION_COUNT, SWORD1_X, SWORD1_Y, single_player_option_button_variant_array, single_player_option_box_array);
	// {
	// 	{ 2+SWORD1_X,  10+SWORD1_Y, 286+SWORD1_X,  65+SWORD1_Y },
	// 	{ 2+SWORD1_X,  67+SWORD1_Y, 286+SWORD1_X, 109+SWORD1_Y },
	// 	{ 2+SWORD1_X, 112+SWORD1_Y, 286+SWORD1_X, 171+SWORD1_Y },
	// 	{ 2+SWORD1_X, 112+SWORD1_Y, 286+SWORD1_X, 171+SWORD1_Y },       // not used
	// 	{38+SWORD1_X, 174+SWORD1_Y, 256+SWORD1_X, 216+SWORD1_Y },
	// };

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
		vga.flip();
		mouse.get_event();

		if( sys.signal_exit_flag == 1 )
		{
			break;
		}

		if( refreshFlag )
		{
			image_interface.put_to_buf( &vga_back, "M_MAIN" );

			vga_util.blt_buf(0,0,VGA_WIDTH-1, VGA_HEIGHT-1);

			if(!menuBitmap)
				menuBitmap = get_bitmap_by_name(BITMAP_SWORD_SINGLEPLAYER_VAL[BITMAP_SWORD::IDLE]);
			
			if(!brightBitmap)
				brightBitmap = get_bitmap_by_name(BITMAP_SWORD_SINGLEPLAYER_VAL[BITMAP_SWORD::HOVER]);
				
			if(!darkBitmap)
				darkBitmap = get_bitmap_by_name(BITMAP_SWORD_SINGLEPLAYER_VAL[BITMAP_SWORD::ACTIVE]);

			for( i = 0; i < SINGLE_PLAYER_OPTION_COUNT; ++i )
			{
				if( single_player_option_flag[i] >= 0)
					update_main_menu_button(SWORD1_X, SWORD1_Y, single_player_option_array[i], single_player_option_flag[i] ? menuBitmap : darkBitmap);
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
				update_main_menu_button(SWORD1_X, SWORD1_Y, single_player_option_array[i], menuBitmap);
			

			// put new hightlighted option
			i = newPointingOption;
			if( i >= 0 && i < SINGLE_PLAYER_OPTION_COUNT )
				update_main_menu_button(SWORD1_X, SWORD1_Y, single_player_option_array[i], brightBitmap);
			
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
						save_game_array.init("*.SAV");

						if( save_game_array.load_game() == 1)
						{
							sys.set_speed(9, COMMAND_AUTO);
							battle.run_loaded();
							deinit();
						}
						{
							char signalExitFlagBackup = sys.signal_exit_flag;
							sys.signal_exit_flag = 2;
							game.deinit();   // game.deinit() is needed if save_game_array.menu fails
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

	static OptionInfo multi_player_option_array[MULTI_PLAYER_OPTION_COUNT];
	ButtonLocation multi_player_option_box_array[MULTI_PLAYER_OPTION_COUNT] = {{6, 10}, {6, 5}, {6, 8}, {6, 8}, {40, 15}};
	static int multi_player_option_button_variant_array[MULTI_PLAYER_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SHORT_SWORD};
	get_main_menu_button_list(multi_player_option_array, MULTI_PLAYER_OPTION_COUNT, SWORD1_X, SWORD1_Y, multi_player_option_button_variant_array, multi_player_option_box_array);
	// {
	// 	{ 5+SWORD1_X,  10+SWORD1_Y, 282+SWORD1_X,  62+SWORD1_Y },
	// 	{ 5+SWORD1_X,  67+SWORD1_Y, 282+SWORD1_X, 112+SWORD1_Y },
	// 	{ 5+SWORD1_X, 120+SWORD1_Y, 282+SWORD1_X, 175+SWORD1_Y },
	// 	{ 5+SWORD1_X, 182+SWORD1_Y, 282+SWORD1_X, 223+SWORD1_Y },
	// 	{40+SWORD1_X, 238+SWORD1_Y, 254+SWORD1_X, 280+SWORD1_Y },
	// };

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
		vga.flip();
		mouse.get_event();

		if( sys.signal_exit_flag == 1 )
		{
			break;
		}

		if( refreshFlag )
		{
			image_interface.put_to_buf( &vga_back, "M_MAIN" );

			vga_util.blt_buf(0,0,VGA_WIDTH-1, VGA_HEIGHT-1);

			if(!menuBitmap)
				menuBitmap = get_bitmap_by_name(BITMAP_SWORD2_VAL[BITMAP_SWORD::IDLE]);
			
			if(!brightBitmap)
				brightBitmap = get_bitmap_by_name(BITMAP_SWORD2_VAL[BITMAP_SWORD::HOVER]);
				
			if(!darkBitmap)
				darkBitmap = get_bitmap_by_name(BITMAP_SWORD_VAL[BITMAP_SWORD::ACTIVE]);


			for( i = 0; i < MULTI_PLAYER_OPTION_COUNT; ++i )
			{
				if( multi_player_option_flag[i] >= 0 )
					update_main_menu_button(SWORD1_X, SWORD1_Y, multi_player_option_array[i], multi_player_option_flag[i] ? menuBitmap : darkBitmap);
				
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
				update_main_menu_button(SWORD1_X, SWORD1_Y, multi_player_option_array[i], menuBitmap);
			

			// put new hightlighted option
			i = newPointingOption;
			if( i >= 0 && i < MULTI_PLAYER_OPTION_COUNT )
				update_main_menu_button(SWORD1_X, SWORD1_Y, multi_player_option_array[i], brightBitmap);
			
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
							save_game_array.init("*.SVM");
							if( save_game_array.menu(2, &loadedRecno) == 1 )
							{
								err_when( !loadedRecno );
								// ####### begin Gilbert 13/2 #######//
								load_mp_game(save_game_array[loadedRecno]->file_info.name, lobbied, game_host);
								// ####### begin Gilbert 13/2 #######//
							}
							{
								char signalExitFlagBackup = sys.signal_exit_flag;
								sys.signal_exit_flag = 2;
								game.deinit();		// game.deinit() is needed if save_game_array.menu fails
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

