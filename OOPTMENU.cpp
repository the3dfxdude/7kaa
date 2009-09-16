// Filename    : OOPTMENU.CPP
// Description : in-game option menu (async version)

#include <OVGA.h>
#include <OVGABUF.h>
#include <OINFO.h>
#include <OCOLTBL.h>
#include <ORACERES.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <KEY.h>
#include <OPOWER.h>
#include <OIMGRES.h>
#include <OFONT.h>
#include <OAUDIO.h>
#include <OMUSIC.h>
#include <OSYS.h>
#include <OOPTMENU.h>


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
#define IGOPTION_MAP_ID          0x00000400
#define IGOPTION_PAGE            0x40000000
#define IGOPTION_ALL             0x7FFFFFFF



OptionMenu::OptionMenu() : help_group(3), news_group(2), report_group(2) ,
	show_icon_group(2), show_path_group(4)

{
	active_flag = 0;
}


void OptionMenu::enter(char untilExitFlag)
{
	if( is_active() )
		return;

	int i;
	refresh_flag = IGOPTION_ALL;
	update_flag = 0;
	active_flag = 1;

	info.save_game_scr();

	Config& tempConfig = config;
	old_config = config;

	// -------- initialize sound effect volume --------//
	se_vol_slide.init_slide(264, 123, 420, 123+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	se_vol_slide.set(0, 100, tempConfig.sound_effect_flag ? tempConfig.sound_effect_volume : 0);

	// -------- initialize music volume --------//
	music_vol_slide.init_slide(566, 123, 722, 123+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	music_vol_slide.set(0, 100, tempConfig.music_flag ? tempConfig.wav_music_volume : 0);

	// -------- initialize frame speed volume --------//
	frame_speed_slide.init_slide(196, 410, 352, 410+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	frame_speed_slide.set(0, 31, tempConfig.frame_speed <= 30 ? tempConfig.frame_speed: 31);
	// use frame 31 to represent full speed (i.e. 99)

	// -------- initialize scroll speed volume --------//
	scroll_speed_slide.init_slide(196, 454, 352, 454+SLIDE_BUTTON_HEIGHT-1, 
		SLIDE_BUTTON_WIDTH, disp_slide_bar);
	scroll_speed_slide.set(0, 10, tempConfig.scroll_speed );

	// --------- initialize race buttons ---------- //

	for( i = 0; i < MAX_RACE; ++i )
	{
		race_button[i].create(181+i*BASIC_OPTION_X_SPACE, 162,
			181+(i+1)*BASIC_OPTION_X_SPACE-1, 162+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, race_table[i]));
	}

	// --------- initialize help button group ---------- //

	for( i = 0; i < 3; ++i )
	{
		help_group[i].create(120+i*BASIC_OPTION_X_SPACE, 244,
			120+(i+1)*BASIC_OPTION_X_SPACE-1, 244+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&help_group, i), 0, 0);
	}

	// --------- initialize news button group ---------- //

	for( i = 0; i < 2; ++i )
	{
		news_group[i].create(198+i*BASIC_OPTION_X_SPACE, 320,
			198+(i+1)*BASIC_OPTION_X_SPACE-1, 320+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&news_group, 1-i), 0, 0);
	}

	// --------- initialize report button group ---------- //

	for( i = 0; i < 2; ++i )
	{
		report_group[i].create(572+i*BASIC_OPTION_X_SPACE, 244,
			572+(i+1)*BASIC_OPTION_X_SPACE-1, 244+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&report_group, 1-i), 0, 0);
	}

	// --------- initialize show icon button group ---------- //

	for( i = 0; i < 2; ++i )
	{
		show_icon_group[i].create(572+i*BASIC_OPTION_X_SPACE, 320,
			572+(i+1)*BASIC_OPTION_X_SPACE-1, 320+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&show_icon_group, 1-i), 0, 0);
	}

	// --------- initialize show path button group ---------- //

	for( i = 0; i < 4; ++i )
	{
		show_path_group[i].create(572+(i/2)*BASIC_OPTION_X_SPACE, 408+(i%2)*BASIC_OPTION_HEIGHT,
			572+(i/2+1)*BASIC_OPTION_X_SPACE-1, 408+(i%2+1)*BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&show_path_group, i), 0, 0);
	}

	// --------- other buttons --------//
	start_button.create(200, 520, "RETURN-U", "RETURN-D", 1, 0);
	cancel_button.create(416, 520, "CANCEL-U", "CANCEL-D", 1, 0);

	mouse_cursor.set_icon(CURSOR_NORMAL);

   power.win_opened = 1;

	if( untilExitFlag )
	{
		while( is_active() )
		{
			sys.yield();
			mouse.get_event();
			disp();
			sys.blt_virtual_buf();
			music.yield();
			detect();
		}
	}
}


void OptionMenu::disp(int needRepaint)
{
	if( !active_flag )
		return;

	if( needRepaint )
		refresh_flag = IGOPTION_ALL;

	int i;
	Config &tempConfig = config;

	// ------- display --------//
	if(refresh_flag)
	{
		if( refresh_flag & IGOPTION_PAGE )
		{
			image_interface.put_to_buf( &vga_back, "OPTIONS");
			vga.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);

			start_button.paint();
			cancel_button.paint();
		}

		if( refresh_flag & IGOPTION_SE_VOL )
		{
			se_vol_slide.paint(tempConfig.sound_effect_flag ? percent_to_slide_volume(tempConfig.sound_effect_volume) : 0);
		}
		if( refresh_flag & IGOPTION_MUSIC_VOL )
		{
			music_vol_slide.paint(tempConfig.music_flag ? percent_to_slide_volume(tempConfig.wav_music_volume) : 0);
		}
		if( refresh_flag & IGOPTION_RACE )
		{
			for( i = 0; i < MAX_RACE; ++i )
				race_button[i].paint();
		}
		if( refresh_flag & IGOPTION_HELP )
		{
			help_group.paint(tempConfig.help_mode);
		}
		if( refresh_flag & IGOPTION_NEWS )
		{
			news_group.paint(1-tempConfig.disp_news_flag);
		}
		if( refresh_flag & IGOPTION_GAME_SPEED )
		{
			frame_speed_slide.paint(tempConfig.frame_speed <= 30 ? tempConfig.frame_speed: 31);
		}
		if( refresh_flag & IGOPTION_SCROLL_SPEED )
		{
			scroll_speed_slide.paint(tempConfig.scroll_speed);
		}
		if( refresh_flag & IGOPTION_REPORT )
		{
			report_group.paint(1-tempConfig.opaque_report);
		}
		if( refresh_flag & IGOPTION_SHOW_ICON )
		{
			show_icon_group.paint(1-tempConfig.show_all_unit_icon);
		}
		if( refresh_flag & IGOPTION_DRAW_PATH )
		{
			show_path_group.paint(tempConfig.show_unit_path);
		}
		if( refresh_flag & IGOPTION_MAP_ID )
		{
			//int x2;
			//x2 = font_san.put(521, 64, "Map ID : ", 1);
			//x2 = font_san.put(x2, 64, info.random_seed, 1);
		}

		refresh_flag = 0;
	}
}



int OptionMenu::detect()
{
	if( !active_flag )
		return 0;

	Config &tempConfig = config;

	int i;
	int retFlag1 = 1;

	if( se_vol_slide.detect() == 1)
	{
		tempConfig.sound_effect_flag = se_vol_slide.view_recno > 0;
		if( se_vol_slide.view_recno > 0)
			tempConfig.sound_effect_volume = slide_to_percent_volume(se_vol_slide.view_recno);
		else
			tempConfig.sound_effect_volume = 1;		// never set sound_effect_volume = 0
		audio.set_wav_volume(tempConfig.sound_effect_volume);

		// change music volume, sound effect volume may change music volume
		if( tempConfig.music_flag )
		{
			music.change_volume( tempConfig.wav_music_volume);
		}
	}
	else if( music_vol_slide.detect() == 1)
	{
		tempConfig.music_flag = music_vol_slide.view_recno > 0;
		tempConfig.wav_music_volume = slide_to_percent_volume(music_vol_slide.view_recno);
		if( tempConfig.music_flag )
		{
			music.change_volume( tempConfig.wav_music_volume );
		}
	}
	else if( frame_speed_slide.detect() == 1)
	{
		tempConfig.frame_speed = frame_speed_slide.view_recno <= 30 ? frame_speed_slide.view_recno : 99;
	}
	else if( scroll_speed_slide.detect() == 1)
	{
		tempConfig.scroll_speed = scroll_speed_slide.view_recno;
	}
	else
		retFlag1 = 0;

	int retFlag2 = 0;
	for( i = 0; i < MAX_RACE; ++i )
	{
		if( race_button[i].detect() )
		{
			if( config.music_flag )
			{
				music.play( race_button[i].custom_para.value + 1,
					sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
			}
			else
			{
				// stop any music playing
				music.stop();
			}
			retFlag2 = 1;
		}
	}

	int retFlag3 = 1;
	if( help_group.detect() >= 0)
	{
		tempConfig.help_mode = help_group[help_group()].custom_para.value;
		//refresh_flag |= IGOPTION_HELP;
		update_flag |= IGOPTION_HELP;
	}
	else if( news_group.detect() >= 0)
	{
		tempConfig.disp_news_flag = news_group[news_group()].custom_para.value;
		//refresh_flag |= IGOPTION_HELP;
		update_flag |= IGOPTION_HELP;
	}
	else if( report_group.detect() >= 0)
	{
		tempConfig.opaque_report = report_group[report_group()].custom_para.value;
		//refresh_flag |= IGOPTION_REPORT;
		update_flag |= IGOPTION_REPORT;
	}
	else if( show_icon_group.detect() >= 0)
	{
		tempConfig.show_all_unit_icon = show_icon_group[show_icon_group()].custom_para.value;
		//refresh_flag |= IGOPTION_SHOW_ICON;
		update_flag |= IGOPTION_SHOW_ICON;
	}
	else if( show_path_group.detect() >= 0)
	{
		tempConfig.show_unit_path = show_path_group[show_path_group()].custom_para.value;
		//refresh_flag |= IGOPTION_DRAW_PATH;
		update_flag |= IGOPTION_DRAW_PATH;
	}
	else if( start_button.detect(KEY_RETURN) )
	{
		if( &config != &tempConfig)
			config = tempConfig;

		exit(1);

		if( update_flag )
		{
			// save config
			Config fileConfig;
			if( !fileConfig.load("CONFIG.DAT") )
				fileConfig.init();
			fileConfig.change_preference(tempConfig);
			fileConfig.save("CONFIG.DAT");
		}

	}
	else if( cancel_button.detect(KEY_ESC) )
	{
		config = old_config;

		exit(0);
	}
	else
		retFlag3 = 0;

	return retFlag1 || retFlag2 || retFlag3;
}

// 0 = abort
// 1 = accept
void OptionMenu::exit(int action)
{

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

	// temporary disable active_flag for info.rest_game_scr to call info.disp
	active_flag = 0;
	info.rest_game_scr();
	active_flag = 1;

   power.win_opened = 0;
	active_flag = 0;

	return;
}


void OptionMenu::abort()
{
	info.rest_game_scr();
   power.win_opened = 0;
	active_flag = 0;
}

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


int OptionMenu::slide_to_percent_volume(int slideVolume)
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

int OptionMenu::percent_to_slide_volume(int percentVolume)
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
