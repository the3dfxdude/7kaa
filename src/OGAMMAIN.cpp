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
#include <OBUTTCUS.h>
// #include <ui/ButtonHoverable.h>

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

enum MENU_TYPE
{
	MAIN_MENU,
	SINGLEPLAYER,
	MULTIPLAYER
};
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

const char *BITMAP_SWORD_VAL_C[][3] = 
{
		{ BITMAP_SWORD_VAL[0], BITMAP_SWORD_VAL[1], BITMAP_SWORD_VAL[2] },
		{ BITMAP_SWORD_SINGLEPLAYER_VAL[0], BITMAP_SWORD_SINGLEPLAYER_VAL[1], BITMAP_SWORD_SINGLEPLAYER_VAL[2] },
		{ BITMAP_SWORD2_VAL[0], BITMAP_SWORD2_VAL[1], BITMAP_SWORD2_VAL[2]}
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
 * @brief Handles the button updates
 *
 * @param button Current reference
 * @param i Additional parameter that could inform that the button is highlighted (2)
 */
static void disp_virtual_button(ButtonCustom *button, int i);
/**
 * @brief Get the bitmap from the name
 *
 * @param bitmap_name
 * @return char* The pointer to the bitmap
 */
char *get_bitmap_by_name(const char *bitmap_name);

void setup_button_list(int start_x, int start_y,
											 ButtonCustom *service_button_list,
											 int count,
											 ButtonLocation button_box_list[],
											 int button_variant_list[],
											 MENU_TYPE menu_type)
{

	int b, button_variant_index;
	int currentX = 0,
			currentY = start_y;

	for (b = 0; b < count; b++)
	{
		button_variant_index = button_variant_list[b];
		err_when(button_variant_index > SWORD_BUTTON_INSTANCES_SIZE);
		currentX = start_x + button_box_list[b].margin_left;
		currentY += button_box_list[b].margin_top;
		const char *button_param = nullptr;
		switch (menu_type)
		{
		case MENU_TYPE::MAIN_MENU :
			button_param = "0";
			break;
		case MENU_TYPE::SINGLEPLAYER :
			button_param = "1";
			break;
		case MENU_TYPE::MULTIPLAYER :
			button_param = "2";
			break;
		default:
			break;
		}
		char value = menu_type + '0';
		const char values[] = {value, '\0'};
		// button_param = &values[0];
		// sprintf(button_param, "%d", menu_type + '0');
		printf("Are equal? %d\n", strncmp(button_param, values, 2));
		printf("button_param : %d, 0x%x, relative addr: %d, addr: %p \n", *button_param, *button_param, button_param, button_param);
		printf("Size of - button_param : %d, relative addr: %d \n", sizeof(*button_param), sizeof(button_param));
		for(int index = 0; index < sizeof(value); index++)
		{
				printf("byte %d - 0x%02hhx\n", index, button_param[index]);
		}
		service_button_list[b]
				.create(currentX, currentY,
								SWORD_BUTTON_INSTANCES[button_variant_index].width + currentX, SWORD_BUTTON_INSTANCES[button_variant_index].height + currentY,
								disp_virtual_button, ButtonCustomPara((void*) button_param, 0), 0);
		currentY += SWORD_BUTTON_INSTANCES[button_variant_index].height;
	}
}

/**
 * @brief Iterate over the list and check if there is any button under the pointer (or anything else maybe)
 *
 * @param button_list An array of buttons
 * @param count The size of the array
 * @param last_active The latest detected button index (from the array) on which the pointer was over
 * @return The current new pointing button index
 */
int update_hover_button(ButtonCustom *button_list, int count, int last_active ){
		int newPointingOption = -1;
		for (int i = 0; i < count; ++i)
		{
			if( button_list[i].detect_hover())
			{
				newPointingOption = i;
				break;
			}
		}

		if(last_active != newPointingOption)
		{
			// put un-highlighted option back
			if( last_active >= 0 && last_active < count )
				button_list[last_active].paint(0);

			// put new hightlighted option
			if( newPointingOption >= 0 && newPointingOption < count )
				button_list[newPointingOption].paint(0, 2);

			return newPointingOption;
		}
		return last_active;
}

static void disp_virtual_button(ButtonCustom *button, int i)
{
	// run_main_menu_option(i+1)
	int x = SWORD1_X,
			y = SWORD1_Y;
	const char * pointer = nullptr;
	pointer = (const char *)button->custom_para.ptr;
	int menu_type = (*pointer) - '0';
	err_when(menu_type > 2 || menu_type < 0);
	// const char (**bitmap_list) = BITMAP_SWORD_VAL;
	// printf("bitmap_list %d %d \n", sizeof((char**)(button->custom_para).ptr), sizeof(menu_type));

  // printf("menu_type %d 0x%x\n", menu_type, menu_type);
	// printf("addr button->custom_para.ptr %p\n", button->custom_para.ptr);
	// printf("* button->custom_para.ptr %d\n", (int *)button->custom_para.ptr);
	// printf("Size of - menu_type : %d, * button->custom_para.ptr %d\n", sizeof(menu_type), sizeof((int *)button->custom_para.ptr));
	
	// for(int index = 0; index < sizeof(menu_type); index++)
	// {
	// 		printf("byte %d - 0x%02hhx\n", index, ((char*)button->custom_para.ptr)[index]);
	// }


	// int i_menu = menu_type;
	const char(**current_bitmap) = BITMAP_SWORD_VAL_C[menu_type];

	// Pushed or highlighted or normal
	char *bitmap = get_bitmap_by_name(button->pushed_flag ? current_bitmap[BITMAP_SWORD::ACTIVE] : i == 2 ? current_bitmap[BITMAP_SWORD::HOVER]
																																																							: current_bitmap[BITMAP_SWORD::IDLE]);

	mouse.hide_area(button->x1, button->y1, button->x2, button->y2);
	vga_front.put_bitmap_area(x, y,
															bitmap,
															button->x1 - x, button->y1 - y,
															button->x2 - x, button->y2 - y);
	mem_del(bitmap);
	mouse.show_area();
}

char * get_bitmap_by_name(const char* bitmap_name){
	char *bitmap = NULL;
	int resSize;
	File *resFile;
	resFile = image_interface.get_file(bitmap_name, resSize);
	bitmap = mem_add(resSize);
	resFile->file_read(bitmap, resSize);
	return bitmap;
}

//---------- Begin of function Game::main_menu ----------//
//
void Game::main_menu()
{
	enum { MAIN_OPTION_COUNT = 6 };
	ButtonCustom main_menu_button_list[MAIN_OPTION_COUNT];
	ButtonLocation option_box_array[MAIN_OPTION_COUNT] = {
		{6, 11}, {6, 5}, {6, 8}, {6, 8}, {6, 2}, {40, 2}
	};
	int button_variant_array[MAIN_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SWORD5, SHORT_SWORD};
	setup_button_list(SWORD1_X, SWORD1_Y,
										main_menu_button_list,
										MAIN_OPTION_COUNT,
										option_box_array, button_variant_array, MENU_TYPE::MAIN_MENU);

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
	int pointingOption = -1;

	for (i = 0; i < MAIN_OPTION_COUNT; i++)
	{
		if(main_option_flag[i] == -1)
			main_menu_button_list[i].disable();
	}

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

			for( i = 0; i < MAIN_OPTION_COUNT; ++i )
			{
				if (main_menu_button_list[i].enable_flag == 1)
					main_menu_button_list[i].paint(0);
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

		pointingOption = update_hover_button(main_menu_button_list, MAIN_OPTION_COUNT, pointingOption);
		// ######### end Gilbert 23/7 ##########//

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		//---------- detect buttons -----------//

		sys.yield();
		vga.flip();
		mouse.get_event();

		// Reset exit-to-main-menu flag
		if (sys.signal_exit_flag == 2) sys.signal_exit_flag = 0;
		
		if(mouse.single_click(0, 0, VGA_WIDTH, VGA_HEIGHT))
		{
			for (i = 0; i < MAIN_OPTION_COUNT; i++)
			{

				if (main_menu_button_list[i].pushed_flag == 0 &&
						main_menu_button_list[i].detect() == 1)
				{
					main_menu_button_list[i].paint(1);
					run_main_menu_option(i+1);
					refreshFlag = 1;
					break;
				}
			}
		}

		

		// method to start a game replay at this time
		if( mouse.is_key_event() && mouse.scan_code == 'r')
		{
			battle.run_replay();
			refreshFlag=1;
		}

		//-------------------------------------//
		if( sys.signal_exit_flag == 1 || main_menu_button_list[MAIN_OPTION_COUNT-1].pushed_flag == 1 )			// quit the system now
			break;
	}

	//--------------------------------------//

	music.stop();
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
	ButtonCustom button_list[SINGLE_PLAYER_OPTION_COUNT];
	
#ifndef DISABLE_SINGLE_PLAYER_NEW_GAME
	ButtonLocation single_player_option_box_array[SINGLE_PLAYER_OPTION_COUNT] = {{6, 10}, {6, 5}, {6, 8}, {6, 8}, {40, 15}};
	static int single_player_option_button_variant_array[SINGLE_PLAYER_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SHORT_SWORD};
	setup_button_list(SWORD1_X, SWORD1_Y,
										button_list,
										SINGLE_PLAYER_OPTION_COUNT,
										single_player_option_box_array,
										single_player_option_button_variant_array,
										MENU_TYPE::SINGLEPLAYER);

	static char single_player_option_flag[SINGLE_PLAYER_OPTION_COUNT] =
	{
		1, 1, 1, 1, 1,				// 1 = in use, 0 = darken, -1 = invisible
	};
#else
	ButtonLocation single_player_option_box_array[SINGLE_PLAYER_OPTION_COUNT] = {{2, 10}, {2, 5}, {2, 3}, {2, -55}, {38, 4}};
	static int single_player_option_button_variant_array[SINGLE_PLAYER_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SHORT_SWORD};
	setup_button_list(SWORD1_X, SWORD1_Y,
										button_list,
										SINGLE_PLAYER_OPTION_COUNT,
										single_player_option_box_array,
										single_player_option_button_variant_array,
										MENU_TYPE::SINGLEPLAYER);

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

			for( i = 0; i < SINGLE_PLAYER_OPTION_COUNT; ++i )
			{
				if (button_list[i].enable_flag == 1)
					button_list[i].paint(0);
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
		pointingOption = update_hover_button(button_list, SINGLE_PLAYER_OPTION_COUNT, pointingOption);

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		if(!mouse.single_click(0, 0, VGA_WIDTH, VGA_HEIGHT))
			continue;

		for( int i=0 ; i<SINGLE_PLAYER_OPTION_COUNT ; i++)
		{
			if (button_list[i].enable_flag &&
					button_list[i].pushed_flag == 0 &&
					button_list[i].detect() == 1)
			{

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
	ButtonCustom button_list[MULTI_PLAYER_OPTION_COUNT];
	ButtonLocation multi_player_option_box_array[MULTI_PLAYER_OPTION_COUNT] = {
		{6, 10}, {6, 5}, {6, 8}, {6, 8}, {40, 15}
	};
	int multi_player_option_button_variant_array[MULTI_PLAYER_OPTION_COUNT] = {SWORD1, SWORD2, SWORD3, SWORD4, SHORT_SWORD};
	setup_button_list(SWORD1_X, SWORD1_Y,
										button_list,
										MULTI_PLAYER_OPTION_COUNT,
										multi_player_option_box_array,
										multi_player_option_button_variant_array,
										MENU_TYPE::MULTIPLAYER);

	static char multi_player_option_flag[MULTI_PLAYER_OPTION_COUNT] =
	{
		-1, 1, 1, -1, 1,
	};

	game_mode = GAME_MULTI_PLAYER;

	//------ display the multi player menu options ------//

	int refreshFlag = 1, i;
	mouse_cursor.set_icon(CURSOR_NORMAL);
	int pointingOption = -1;

	for (i = 0; i < MULTI_PLAYER_OPTION_COUNT; i++)
	{
		if(multi_player_option_flag[i] == -1)
			button_list[i].disable();
	}

	//---------- detect buttons -----------//
	while (1)
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

			for( i = 0; i < MULTI_PLAYER_OPTION_COUNT; ++i )
			{
				if (button_list[i].enable_flag == 1)
					button_list[i].paint(0);
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
		pointingOption = update_hover_button(button_list, MULTI_PLAYER_OPTION_COUNT, pointingOption);

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen
		if(!mouse.single_click(0, 0, VGA_WIDTH, VGA_HEIGHT))
			continue;

		for( int i=0 ; i<MULTI_PLAYER_OPTION_COUNT ; i++ )
		{
			if (button_list[i].enable_flag &&
					button_list[i].pushed_flag == 0 &&
					button_list[i].detect() == 1)
			{
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
}
//------------ End of function Game::multi_player_menu -----------//

#endif

