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

//Filename   : OGAMMENU.CPP
//Description: In Game Menu

#include <KEY.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OPOWER.h>
#include <OBOX.h>
#include <OFONT.h>
#include <OIMGRES.h>
#include <OINFO.h>
#include <OGAME.h>
#include <OGFILE.h>
#include <OCONFIG.h>
#include <ORACERES.h>
#include <OBUTT3D.h>
#include <OBUTTCUS.h>
#include <OCOLTBL.h>
#include <OMUSIC.h>
#include <OSLIDCUS.h>
#include <OAUDIO.h>
#include <OREMOTE.h>
#include <OTUTOR.h>
#include <OOPTMENU.h>


//--------- Define Constant ----------//

enum { GAME_MENU_WIDTH  = 350,
		 GAME_MENU_HEIGHT = 386  };

enum { GAME_MENU_X1 = ZOOM_X1 + ( (ZOOM_X2-ZOOM_X1+1) - GAME_MENU_WIDTH ) / 2,
		 GAME_MENU_Y1 = ZOOM_Y1 + ( (ZOOM_Y2-ZOOM_Y1+1) - GAME_MENU_HEIGHT ) / 2 };

enum { GAME_OPTION_WIDTH  = 170,
		 GAME_OPTION_HEIGHT = 34   };

// ####### begin Gilbert 29/10 #########//
enum { GAME_OPTION_X1 = GAME_MENU_X1+90,
		 GAME_OPTION_Y1 = GAME_MENU_Y1+76  };

enum { MAP_ID_X1 = GAME_MENU_X1 + 18, 
		 MAP_ID_Y1 = GAME_MENU_Y1 + 352 };
// ####### end Gilbert 29/10 #########//

enum { GAME_OPTION_COUNT = 8 };

static char game_menu_option_flag[GAME_OPTION_COUNT];
static unsigned menu_hot_key[GAME_OPTION_COUNT] = {'o','s','l', 0,0,0,0,KEY_ESC };
//------- Define static functions -------//

static void init_game_menu_option_flag();
static int detect_game_option();

static int slide_to_percent_volume(int);
static int percent_to_slide_volume(int);

//------- Begin of function Game::in_game_menu -------//

void Game::in_game_menu()
{
	int x=GAME_MENU_X1+20, y=GAME_MENU_Y1+17;

//	vga_back.adjust_brightness( x, y, x+GAME_MENU_WIDTH-1, y+GAME_MENU_HEIGHT-1, -6 );
//	vga.blt_buf( x, y, x+GAME_MENU_WIDTH-1, y+GAME_MENU_HEIGHT-1, 0 );

	image_interface.put_front( GAME_MENU_X1, GAME_MENU_Y1, "GAMEMENU" );
	init_game_menu_option_flag();
	for( int b = 0; b < GAME_OPTION_COUNT; ++b)
	{
		if( !game_menu_option_flag[b])
		{
			// darked disabled button
			vga_front.adjust_brightness(
			GAME_OPTION_X1, GAME_OPTION_Y1 + b*GAME_OPTION_HEIGHT,
			GAME_OPTION_X1+GAME_OPTION_WIDTH-1, 
			GAME_OPTION_Y1 + (b+1)*GAME_OPTION_HEIGHT-1, -8);
		}
	}

	// ####### begin Gilbert 29/10 #########//
	int textX;
	textX = font_bible.put( MAP_ID_X1, MAP_ID_Y1, "Map Id : ");
	textX = font_bible.put( textX, MAP_ID_Y1, info.random_seed);
	// ####### end Gilbert 29/10 #########//

	mouse_cursor.set_icon(CURSOR_NORMAL);

   power.win_opened = 1;

	while(1)
	{
		//---------- yield --------//

		sys.yield();

		mouse.get_event();

		sys.blt_virtual_buf();

		// ------- play music --------//
		int oldSongId = music.song_id;
		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(music.random_bgm_track(oldSongId), sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
		{
			// stop any music playing
			music.stop();
		}

		//----- detect options ------//

		if( detect_game_option() )
			break;

		//----- right click or ESC to quit -----//

		if( mouse.any_click(1) || mouse.key_code==KEY_ESC )
			break;
	}

	power.win_opened = 0;
}
//-------- End of function Game::in_game_menu --------//

// ------- Begin of static function init_game_menu_option_flag -----//
static void init_game_menu_option_flag()
{
	memset(game_menu_option_flag, 1, sizeof(game_menu_option_flag) );

	if( !nation_array.player_recno)
	{
		// when in observe mode
		game_menu_option_flag[1] = 0;		// disable save game
		game_menu_option_flag[4] = 0;		// disable retire
	}

	if( remote.is_enable() )
	{
		// when in when in multi-player mode,
		game_menu_option_flag[2] = 0;		// disable load game
		game_menu_option_flag[3] = 0;		// disable training
		game_menu_option_flag[4] = 0;		// disable retire
	}
}
// ------- End of static function init_game_menu_option_flag -----//

//------- Begin of static function detect_game_option -------//

static int detect_game_option()
{
	int i, y=GAME_OPTION_Y1, x2, y2;

	for( i=1 ; i<=GAME_OPTION_COUNT ; i++, y+=GAME_OPTION_HEIGHT )
	{
		x2 = GAME_OPTION_X1+GAME_OPTION_WIDTH-1;
		y2 = y+GAME_OPTION_HEIGHT-1;

		if( game_menu_option_flag[i-1] == 1 && 
			(menu_hot_key[i-1] && mouse.key_code == menu_hot_key[i-1] ||
			mouse.single_click( GAME_OPTION_X1, y, x2, y2 )) )
			break;
	}

	if( i>GAME_OPTION_COUNT )
		return 0;

	//------ display the pressed down button -----//

	vga_front.save_area_common_buf( GAME_OPTION_X1, y, x2, y2 );

//	image_interface.put_front( GAME_OPTION_X1, y, "GAM_DOWN" );
	image_interface.put_front( GAME_OPTION_X1, y, "MENU-DWN" );

	while( mouse.left_press )	// holding down the button
	{
		sys.yield();
		mouse.get_event();
	}

	vga_front.rest_area_common_buf();			// restore the up button

	//--------- run the option -------//

	switch(i)
	{
		case 1:		// options
			// game.in_game_option_menu();
			option_menu.enter(!remote.is_enable());
			break;

		case 2:		// save game
			sys.save_game();
			break;

		case 3:		// load game
			sys.load_game();
			break;

		case 4:		// training
			tutor.select_run_tutor(1);
			break;

		case 5:		// retire
			if( nation_array.player_recno )		// only when the player's kingdom still exists
			{
				if( box.ask("Do you really want to retire?", "Yes", "No", 175, 320) )
					game.game_end(0, 0, 0, 1);				// 1 - retire
			}
			break;

		case 6:		// quit to main menu
			if( !nation_array.player_recno ||
				 box.ask( "Do you really want to quit to the Main Menu?", "Yes", "No", 115, 350 ) )
			{
				if( remote.is_enable() && nation_array.player_recno )
				{
					// BUGHERE : message will not be sent out
					short *shortPtr = (short *)remote.new_send_queue_msg( MSG_PLAYER_QUIT, 2*sizeof(short));
					shortPtr[0] = nation_array.player_recno;
					shortPtr[1] = 0;		// not retire
				}
				sys.signal_exit_flag = 2;
			}
			break;

		case 7:		// quit to Windows
			if( !nation_array.player_recno ||
				 box.ask( "Do you really want to quit to Windows?", "Yes", "No", 130, 400 ) )
			{
				if( remote.is_enable() && nation_array.player_recno )
				{
					// BUGHERE : message will not be sent out
					short *shortPtr = (short *)remote.new_send_queue_msg( MSG_PLAYER_QUIT, 2*sizeof(short));
					shortPtr[0] = nation_array.player_recno;
					shortPtr[1] = 1;		// retire
				}
				sys.signal_exit_flag = 1;
			}
			break;
	}

	return 1;
}
//-------- End of static function detect_game_option -------//

enum { BASIC_OPTION_X_SPACE = 78,
		 BASIC_OPTION_HEIGHT = 32 };

enum { COLOR_OPTION_X_SPACE = 35,
		 COLOR_OPTION_HEIGHT = 32 };

enum { SERVICE_OPTION_X_SPACE = 180,
		 SERVICE_OPTION_HEIGHT = 139 };

enum { SLIDE_BUTTON_WIDTH = 23,
		 SLIDE_BUTTON_HEIGHT = 24 };

static char race_table[MAX_RACE] =		// race translation table
{
	RACE_CHINESE, RACE_GREEK, RACE_JAPANESE, RACE_MAYA,
	RACE_PERSIAN, RACE_NORMAN, RACE_VIKING
};

static char reverse_race_table[MAX_RACE] =		// race translation table
{
	5, 3, 1, 6, 4, 0, 2 
};

static void disp_virtual_button(ButtonCustom *button, int);
static void disp_slide_bar(SlideBar *slideBar, int);

// return 1 if ok, config is changed
#define IGOPTION_SE_VOL          0x00000001
#define IGOPTION_MUSIC_VOL       0x00000002
#define IGOPTION_RACE            0x00000004
#define IGOPTION_HELP            0x00000008
#define IGOPTION_NEWS            0x00000010
#define IGOPTION_GAME_SPEED      0x00000020
#define IGOPTION_SCROLL_SPEED    0x00000040
#define IGOPTION_REPORT          0x00000080
#define IGOPTION_SHOW_ICON       0x00000100
#define IGOPTION_DRAW_PATH       0x00000200
#define IGOPTION_PAGE            0x40000000
#define IGOPTION_ALL             0x7FFFFFFF

// ---------- begin of function Game::in_game_option_menu ------//
int Game::in_game_option_menu()
{
	int i;
	int retFlag = 0;
	int refreshFlag = IGOPTION_ALL;

	info.save_game_scr();

	Config& tempConfig = config;
	Config oldConfig = config;

	// -------- initialize sound effect volume --------//
	SlideBar seVolSlide;
	seVolSlide.init_slide(264, 123, 420, 123+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	seVolSlide.set(0, 100, tempConfig.sound_effect_flag ? tempConfig.sound_effect_volume : 0);

	// -------- initialize music volume --------//
	SlideBar musicVolSlide;
	musicVolSlide.init_slide(566, 123, 722, 123+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	musicVolSlide.set(0, 100, tempConfig.music_flag ? tempConfig.wav_music_volume : 0);

	// -------- initialize frame speed volume --------//
	SlideBar frameSpeedSlide;
	frameSpeedSlide.init_slide(196, 410, 352, 410+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	frameSpeedSlide.set(0, 31, tempConfig.frame_speed <= 30 ? tempConfig.frame_speed: 31);
	// use frame 31 to represent full speed (i.e. 99)

	// -------- initialize scroll speed volume --------//
	SlideBar scrollSpeedSlide;
	scrollSpeedSlide.init_slide(196, 454, 352, 454+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	scrollSpeedSlide.set(0, 10, tempConfig.scroll_speed );

	// --------- initialize race buttons ---------- //

	ButtonCustom raceButton[MAX_RACE];
	for( i = 0; i < MAX_RACE; ++i )
	{
		raceButton[i].create(181+i*BASIC_OPTION_X_SPACE, 162,
			181+(i+1)*BASIC_OPTION_X_SPACE-1, 162+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, race_table[i]));
	}

	// --------- initialize help button group ---------- //

	ButtonCustomGroup helpGroup(3);
	for( i = 0; i < 3; ++i )
	{
		helpGroup[i].create(120+i*BASIC_OPTION_X_SPACE, 244,
			120+(i+1)*BASIC_OPTION_X_SPACE-1, 244+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&helpGroup, i), 0, 0);
	}

	// --------- initialize news button group ---------- //

	ButtonCustomGroup newsGroup(2);
	for( i = 0; i < 2; ++i )
	{
		newsGroup[i].create(198+i*BASIC_OPTION_X_SPACE, 320,
			198+(i+1)*BASIC_OPTION_X_SPACE-1, 320+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&newsGroup, 1-i), 0, 0);
	}

	// --------- initialize report button group ---------- //

	ButtonCustomGroup reportGroup(2);
	for( i = 0; i < 2; ++i )
	{
		reportGroup[i].create(572+i*BASIC_OPTION_X_SPACE, 244,
			572+(i+1)*BASIC_OPTION_X_SPACE-1, 244+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&reportGroup, 1-i), 0, 0);
	}

	// --------- initialize show icon button group ---------- //

	ButtonCustomGroup showIconGroup(2);
	for( i = 0; i < 2; ++i )
	{
		showIconGroup[i].create(572+i*BASIC_OPTION_X_SPACE, 320,
			572+(i+1)*BASIC_OPTION_X_SPACE-1, 320+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&showIconGroup, 1-i), 0, 0);
	}

	// --------- initialize show path button group ---------- //

	ButtonCustomGroup showPathGroup(4);
	for( i = 0; i < 4; ++i )
	{
		showPathGroup[i].create(572+(i/2)*BASIC_OPTION_X_SPACE, 408+(i%2)*BASIC_OPTION_HEIGHT,
			572+(i/2+1)*BASIC_OPTION_X_SPACE-1, 408+(i%2+1)*BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&showPathGroup, i), 0, 0);
	}

	// --------- other buttons --------//
	Button3D startButton, cancelButton;
	startButton.create(200, 520, "RETURN-U", "RETURN-D", 1, 0);
	cancelButton.create(416, 520, "CANCEL-U", "CANCEL-D", 1, 0);

	mouse_cursor.set_icon(CURSOR_NORMAL);

   power.win_opened = 1;

	while(1)
	{
		sys.yield();
		mouse.get_event();

		// ------- display --------//
		if(refreshFlag)
		{
			if( refreshFlag & IGOPTION_PAGE )
			{
				image_interface.put_to_buf( &vga_back, "OPTIONS");
				vga.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);

				startButton.paint();
				cancelButton.paint();
			}

			if( refreshFlag & IGOPTION_SE_VOL )
			{
				seVolSlide.paint(tempConfig.sound_effect_flag ? percent_to_slide_volume(tempConfig.sound_effect_volume) : 0);
			}
			if( refreshFlag & IGOPTION_MUSIC_VOL )
			{
				musicVolSlide.paint(tempConfig.music_flag ? percent_to_slide_volume(tempConfig.wav_music_volume) : 0);
			}
			if( refreshFlag & IGOPTION_RACE )
			{
				for( i = 0; i < MAX_RACE; ++i )
					raceButton[i].paint();
			}
			if( refreshFlag & IGOPTION_HELP )
			{
				helpGroup.paint(tempConfig.help_mode);
			}
			if( refreshFlag & IGOPTION_NEWS )
			{
				newsGroup.paint(1-tempConfig.disp_news_flag);
			}
			if( refreshFlag & IGOPTION_GAME_SPEED )
			{
				frameSpeedSlide.paint(tempConfig.frame_speed <= 30 ? tempConfig.frame_speed: 31);
			}
			if( refreshFlag & IGOPTION_SCROLL_SPEED )
			{
				scrollSpeedSlide.paint(tempConfig.scroll_speed);
			}
			if( refreshFlag & IGOPTION_REPORT )
			{
				reportGroup.paint(1-tempConfig.opaque_report);
			}
			if( refreshFlag & IGOPTION_SHOW_ICON )
			{
				showIconGroup.paint(1-tempConfig.show_all_unit_icon);
			}
			if( refreshFlag & IGOPTION_DRAW_PATH )
			{
				showPathGroup.paint(tempConfig.show_unit_path);
			}

			refreshFlag = 0;
		}


		// ------- play music --------//
		int oldSongId = music.song_id;
		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(music.random_bgm_track(oldSongId), sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
		{
			// stop any music playing
			music.stop();
		}

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		// -------- detect button --------//
		if( seVolSlide.detect() == 1)
		{
			tempConfig.sound_effect_flag = seVolSlide.view_recno > 0;
			if( seVolSlide.view_recno > 0)
				tempConfig.sound_effect_volume = slide_to_percent_volume(seVolSlide.view_recno);
			else
				tempConfig.sound_effect_volume = 1;		// never set sound_effect_volume = 0
			audio.set_wav_volume(tempConfig.sound_effect_volume);

			// change music volume, sound effect volume may change music volume
			if( tempConfig.music_flag )
			{
				music.change_volume( tempConfig.wav_music_volume);
			}
		}
		else if( musicVolSlide.detect() == 1)
		{
			tempConfig.music_flag = musicVolSlide.view_recno > 0;
			tempConfig.wav_music_volume = slide_to_percent_volume(musicVolSlide.view_recno);
			if( tempConfig.music_flag )
			{
				music.change_volume( tempConfig.wav_music_volume );
			}
		}
		else if( frameSpeedSlide.detect() == 1)
		{
			tempConfig.frame_speed = frameSpeedSlide.view_recno <= 30 ? frameSpeedSlide.view_recno : 99;
		}
		else if( scrollSpeedSlide.detect() == 1)
		{
			tempConfig.scroll_speed = scrollSpeedSlide.view_recno;
		}

		for( i = 0; i < MAX_RACE; ++i )
		{
			if( raceButton[i].detect() )
			{
				if( config.music_flag )
				{
					music.play( raceButton[i].custom_para.value + 1,
						sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
				}
				else
				{
					// stop any music playing
					music.stop();
				}
			}
		}

		if( helpGroup.detect() >= 0)
		{
			tempConfig.help_mode = helpGroup[helpGroup()].custom_para.value;
			//refreshFlag |= IGOPTION_HELP;
		}
		else if( newsGroup.detect() >= 0)
		{
			tempConfig.disp_news_flag = newsGroup[newsGroup()].custom_para.value;
			//refreshFlag |= IGOPTION_HELP;
		}
		else if( reportGroup.detect() >= 0)
		{
			tempConfig.opaque_report = reportGroup[reportGroup()].custom_para.value;
			//refreshFlag |= IGOPTION_REPORT;
		}
		else if( showIconGroup.detect() >= 0)
		{
			tempConfig.show_all_unit_icon = showIconGroup[showIconGroup()].custom_para.value;
			//refreshFlag |= IGOPTION_SHOW_ICON;
		}
		else if( showPathGroup.detect() >= 0)
		{
			tempConfig.show_unit_path = showPathGroup[showPathGroup()].custom_para.value;
			//refreshFlag |= IGOPTION_SHOW_PATH;
		}
		else if( startButton.detect(KEY_RETURN) )
		{
			if( &config != &tempConfig)
				config = tempConfig;

			// save config
			Config fileConfig;
		   if( !fileConfig.load("CONFIG.DAT") )
				fileConfig.init();
			fileConfig.change_preference(tempConfig);
			fileConfig.save("CONFIG.DAT");

			retFlag = 1;
			break;
		}
		else if( cancelButton.detect(KEY_ESC) )
		{
			config = oldConfig;
			retFlag = 0;
			break;
		}
	}

	// reflect the effect of config.music_flag, config.wav_music_volume
	audio.set_wav_volume(config.sound_effect_volume);
	if( config.music_flag )
	{
		if( music.is_playing() )
		{
			music.change_volume(config.wav_music_volume);
		}
	}
	else
	{
		music.stop();
	}

	info.rest_game_scr();
   power.win_opened = 0;

	return retFlag;
}
// ---------- end of function Game::in_game_option_menu ------//


// ---------- begin of static function disp_virtual_button -----//
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
		else if( button->x2-button->x1+1 == SERVICE_OPTION_X_SPACE &&
			button->y2-button->y1+1 == SERVICE_OPTION_HEIGHT )
		{
			image_interface.put_front(button->x1, button->y1, "NMPG-1BD");
		}
	}
	mouse.show_area();
}
// ---------- end of static function disp_virtual_button -----//


// ---------- begin of static function disp_slide_bar  -----//
static void disp_slide_bar(SlideBar *slideBar, int)
{
	vga.blt_buf(slideBar->scrn_x1, slideBar->scrn_y1, 
		slideBar->scrn_x2, slideBar->scrn_y2, 0 );

	image_interface.put_front(slideBar->rect_left(), slideBar->scrn_y1, "SLIDBALL");
}
// ---------- end of static function disp_slide_bar  -----//


static int slide_to_percent_volume(int slideVolume)
{
	switch( slideVolume / 10)
	{
	case 0:
		return slideVolume * 5;
	case 1:
	case 2:
	case 3:
		return slideVolume+40;
		break;

	case 4:
	case 5:
		return slideVolume/2 + 60;
		break;

	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		return slideVolume/4+75;
		break;

	default:
		err_here();
		return slideVolume;
	}
}

// slideVolume  0    10   20   30   40   50   60   70   80   90   100
//              !----!----!----!----!----!----!----!----!----!----!
// percentVoume 0    50             80        90                  100

static int percent_to_slide_volume(int percentVolume)
{
	switch(percentVolume/10)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		return percentVolume/5;

	case 5:
	case 6:
	case 7:
		return percentVolume - 40;

	case 8:
		return (percentVolume-60) * 2;

	case 9:
	case 10:
		return (percentVolume-75) * 4;

	default:
		err_here();
		return percentVolume;
	}
}


void Game::disp_gen_map_status( int curStep, int maxStep, int section )
{
	const int POPUP_WINDOW_WIDTH = 266;
	const int POPUP_WINDOW_HEIGHT = 149;
	const int POPUP_WINDOW_X1 = (vga_front.buf_width() - POPUP_WINDOW_WIDTH) / 2;
	const int POPUP_WINDOW_Y1 = (vga_front.buf_height() - POPUP_WINDOW_HEIGHT) / 2;

	const int BAR_X1 = POPUP_WINDOW_X1 + 46;
	// ###### begin Gilbert 29/10 ######//
	const int BAR_Y1 = POPUP_WINDOW_Y1 + 106;
	// ###### end Gilbert 29/10 ######//

	const int MAX_SECTION = 2;		// section 0 for world.genmap, section 1 for battle.run
	static int accSectionWeight[MAX_SECTION+1] = { 0, 60, 100 };

	if( section == 0 && curStep == 0)
	{
		image_menu.put_front(POPUP_WINDOW_X1, POPUP_WINDOW_Y1, "NEWWORLD");
	}

	err_when( section < 0 || section >= MAX_SECTION );
	err_when( curStep < 0 || curStep > maxStep );
	if( curStep >= 0 && curStep <= maxStep)
	{
		float r = float(accSectionWeight[section]) + 
			float((accSectionWeight[section+1]-accSectionWeight[section]) * curStep) / maxStep;
		vga_front.indicator(4, BAR_X1, BAR_Y1, r, (float)accSectionWeight[MAX_SECTION], 0);
	}

	sys.blt_virtual_buf();
}
