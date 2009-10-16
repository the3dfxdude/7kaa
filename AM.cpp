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

//Filename    : AM.CPP
//Description : Ambition Entry Program

#include <windows.h>
#include <initguid.h>
#include <dshow.h>

#include <ALL.h>
#include <OANLINE.h>
#include <OAUDIO.h>
#include <OBATTLE.h>
#include <OBOX.h>
#include <OBULLET.h>
#include <OCONFIG.h>
#include <ODATE.h>
#include <OFIRM.h>
#include <OFLAME.h>
#include <OFONT.h>
#include <OGAME.h>
#include <OGAMESET.h>
#include <OGFILE.h>
#include <OGODRES.h>
#include <OHELP.h>
#include <OHILLRES.h>
#include <OIMGRES.h>
#include <OINFO.h>
#include <OMONSRES.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <ONATION.h>
#include <ONEWS.h>
#include <OPLANT.h>
#include <OPOWER.h>
#include <ORACERES.h>
#include <OREBEL.h>
#include <OREMOTE.h>
#include <OSPATH.h>
#include <OSPATHS2.h>
#include <OSITE.h>
#include <OSPREUSE.h>
#include <OSPY.h>
#include <OSYS.h>
#include <OTALKRES.h>
#include <OTECHRES.h>
#include <OTERRAIN.h>
#include <OTOWN.h>
#include <OTRANSL.h>
#include <OUNIT.h>
#include <OVGA.h>
#include <OVIDEO.h>
#include <OWALLRES.h>
#include <OWORLD.h>
#include <OWEATHER.h>
#include <OTORNADO.h>
#include <OTUTOR.h>
#include <OSE.h>
#include <OSERES.h>
#include <OSNOWRES.h>
#include <OSNOWG.h>
#include <OROCKRES.h>
#include <OROCK.h>
#include <OEFFECT.h>
#include <OEXPMASK.h>
#include <OREGION.h>
#include <OWARPT.h>
#include <ODPLAY.h>
#include <OIMMPLAY.h>
#include <OERRCTRL.h>
#include <OMUSIC.h>
#include <OLOG.h>
#include <OLONGLOG.h>
//### begin alex 3/10 ###//
#include <OGRPSEL.h>
//#### end alex 3/10 ####//
#include <OFIRMDIE.h>
#include <OCRC_STO.h>
// ###### begin Gilbert 23/10 #######//
#include <OOPTMENU.h>
#include <OINGMENU.h>
// ###### end Gilbert 23/10 #######//

//------- define game version constant --------//

#ifdef AMPLUS
	#ifdef IMAGICMP
		char *GAME_VERSION_STR = "2.12b2";
	#else
		char *GAME_VERSION_STR = "2.12b2DX";
	#endif
	const int GAME_VERSION = 212;	// Version 2.00, don't change it unless the format of save game files has been changed
#else
	char *GAME_VERSION_STR = "1.11";
	const int GAME_VERSION = 111;	// Version 1.00, don't change it unless the format of save game files has been changed
#endif

//-------- System class ----------//

#ifndef NO_MEM_CLASS
   Mem   mem;              // constructor only init var and allocate memory
#endif

Error             err;              // constructor only call set_new_handler()d
Mouse             mouse;
MouseCursor       mouse_cursor;
Misc              m, m2;
DateInfo          date;
Vga               vga;
VgaBuf            vga_front, vga_back, vga_true_front;
Video             video;
Audio             audio;
Music             music;
MultiPlayerType	mp_obj;
// MultiPlayerDP	mp_dp;
// MultiPlayerIM	mp_im;
Sys               sys;
Translate         translate;        // constructor only memset()
SeekPath          seek_path;
SeekPathS2        seek_path_s2;
SeekPathReuse     seek_path_reuse;
Flame             flame[FLAME_GROW_STEP];
Remote            remote;
ErrorControl      ec_remote;
AnimLine          anim_line;
SECtrl            se_ctrl(&audio);
SERes             se_res;
Log               msg_log;
#ifdef DEBUG
LongLog *			long_log;
#endif
//### begin alex 3/10 ###//
GroupSelect			group_select;
//#### end alex 3/10 ####//

//------- Resource class ----------//

Font              font_san, font_std, font_small, font_mid, font_news;
Font              font_hitpoint, font_bible, font_bard;

#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
Font					font_hall;
#endif

Box               box;
ImageRes          image_icon, image_interface, image_menu,
						image_button, image_spict;
ImageRes          image_encyc;
ImageRes				image_tpict;
ImageRes				image_tutorial;
#ifdef AMPLUS
ImageRes				image_menu_plus;
ImageRes&			image_menu2 = image_menu_plus;
#else
ImageRes&			image_menu2 = image_menu;
#endif
SpriteRes         sprite_res;
SpriteFrameRes    sprite_frame_res;
UnitRes           unit_res;
TerrainRes        terrain_res;
PlantRes          plant_res;
WallRes           wall_res;
RawRes            raw_res;
FirmRes           firm_res;
FirmDieRes			firm_die_res;
RaceRes           race_res;
TownRes           town_res;
HillRes           hill_res;
TalkRes           talk_res;
TechRes           tech_res;
GodRes            god_res;
MonsterRes        monster_res;
SnowRes           snow_res;
RockRes           rock_res;
ExploredMask      explored_mask;
Help              help;
Tutor             tutor;

//-------- Game Data class -----------//

UnitArray         unit_array(100);        // 100-initial array size
BulletArray       bullet_array(100);
SiteArray         site_array;
TownArray         town_array;
NationArray       nation_array;
FirmArray         firm_array;
FirmDieArray		firm_die_array;
TornadoArray      tornado_array(10);
RebelArray        rebel_array;
SpyArray          spy_array;
SnowGroundArray   snow_ground_array;
RockArray         rock_array;
RockArray         dirt_array;
SpriteArray       effect_array(50);
RegionArray       region_array;
NewsArray         news_array;
WarPointArray     war_point_array;
CrcStore				crc_store;

//--------- Game Surface class ------------//

Info              info;
Weather           weather, weather_forecast[MAX_WEATHER_FORECAST];
MagicWeather      magic_weather;
Config            config;
Game              game;
GameSet           game_set;         // no constructor
Battle            battle;
Power             power;
World             world;
GameFileArray     game_file_array;
GameFile          game_file;
// ###### begin Gilbert 23/10 #######//
OptionMenu			option_menu;
InGameMenu			in_game_menu;
// ###### end Gilbert 23/10 #######//

//----------- Global Variables -----------//

char game_design_mode=0;
char game_demo_mode=0;
char debug2_enable_flag=0;
File seedCompareFile;
char debug_seed_status_flag=0;
int  debug_sim_game_type = 0;
int  unit_search_node_used=0;
short nation_hand_over_flag=0;
int     unit_search_tries = 0;        // the number of tries used in the current searching
char    unit_search_tries_flag = 0;   // indicate num of tries is set, reset after searching

char 	new_config_dat_flag=0;

#ifdef DEBUG
int check_unit_dir1, check_unit_dir2;
unsigned long	last_unit_ai_profile_time = 0L;
unsigned long	unit_ai_profile_time = 0L;
unsigned long	last_unit_profile_time = 0L;
unsigned long	unit_profile_time = 0L;
unsigned long seek_path_profile_time = 0L;
unsigned long last_seek_path_profile_time = 0L;
unsigned long last_sprite_array_profile_time = 0L;
unsigned long sprite_array_profile_time = 0L;
unsigned long last_sprite_idle_profile_time = 0L;
unsigned long sprite_idle_profile_time = 0L;
unsigned long last_sprite_move_profile_time = 0L;
unsigned long sprite_move_profile_time = 0L;
unsigned long last_sprite_wait_profile_time = 0L;
unsigned long sprite_wait_profile_time = 0L;
unsigned long last_sprite_attack_profile_time = 0L;
unsigned long sprite_attack_profile_time = 0L;
unsigned long last_unit_attack_profile_time = 0L;
unsigned long unit_attack_profile_time = 0L;
unsigned long last_unit_assign_profile_time = 0L;
unsigned long unit_assign_profile_time = 0L;
#endif

//------- Define static functions --------//

static void extra_error_handler();

//---------- Begin of function WinMain ----------//
//¹ç
// WinMain - initialization, message loop
//
// Compilation constants:
//
// DEBUG  - normal debugging
// DEBUG2 - shortest path searching and unit action debugging
// DEBUG3 - debugging some functions (e.g. Location::get_loc()) which
//          will cause major slowdown.
//
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
						  LPSTR lpCmdLine, int nCmdShow)
{
	//try to read from CONFIG.DAT, moved to AM.CPP

	if( !config.load("CONFIG.DAT") )
	{
		new_config_dat_flag = 1;
		config.init();
   }

	//--------------------------------------//

#if(defined(BETA) && defined(AMPLUS))
#define EXPIRE_YEAR 1998
#define EXPIRE_MONTH 4
#define EXPIRE_DAY 30
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	if( config.expired_flag || sysTime.wYear > EXPIRE_YEAR ||
		(sysTime.wYear == EXPIRE_YEAR && (sysTime.wMonth > EXPIRE_MONTH
		|| sysTime.wMonth == EXPIRE_MONTH && sysTime.wDay >= EXPIRE_DAY)) )
	{
		if( !config.expired_flag )
		{
			// write back config with expired_flag set
			config.expired_flag = 1;
			config.save("CONFIG.DAT");
		}
		MessageBox( NULL, "Beta version expired", "Seven Kingdoms",
			MB_OK | MB_ICONSTOP | MB_DEFBUTTON1 | MB_TASKMODAL );
		return 2;
	}
#endif

#ifdef IMAGICMP
	static char lobbyLaunchCmdLine[] = "IM";
#else
	static char lobbyLaunchCmdLine[] = "-!lobby!";
#endif

	//----------- play movie ---------------//

	sys.set_game_dir();
	if( strstr(lpCmdLine, lobbyLaunchCmdLine) == NULL )	// skip if launch from lobby
	{
		String movieFileStr;
		movieFileStr = DIR_MOVIE;
		movieFileStr += "INTRO.AVI";

		video.set_skip_on_fail();

		// ###### begin Gilbert 29/10 #####//
		if( !m.is_file_exist("SKIPAVI.SYS") && m.is_file_exist(movieFileStr) )
		// ###### end Gilbert 29/10 #####//
		{
			//---------- play the movie now ---------//

			video.init();

			if( video.init_success )
			{
				video.play_until_end( movieFileStr, hInstance, 60 );
			}
			else
			{
				// display a message box (note:sys.main_hwnd is not valid)
				// MessageBox( NULL, "Cannot initialize ActiveMovie",
				//   "Seven Kingdoms", MB_OK | MB_ICONWARNING | MB_DEFBUTTON1 | MB_TASKMODAL );
			}

			video.deinit();
		}
	}

   if( !sys.init(hInstance) )
      return FALSE;

   err.set_extra_handler( extra_error_handler );   // set extra error handler, save the game when a error happens

#ifdef DEMO
	game.demo_disp_logo();
   game.main_menu();
#else
	if( strstr(lpCmdLine, lobbyLaunchCmdLine) == NULL )
	   game.main_menu();
#ifndef DISABLE_MULTI_PLAYER
	else
		game.multi_player_menu(lpCmdLine);		// if detect launched from lobby
#endif // DISABLE_MULTI_PLAYER
#endif

   sys.deinit();

   return 1;
}
//---------- End of function WinMain ----------//


//----------- Begin of function Msg ----------//
//
// Routine for displaying debug messages
//

#ifdef DEBUG

void __cdecl debug_msg( char* fmt, ... )
{
    char buff[256];

    lstrcpy( buff, "SEVEN KINGDOMS: " );
    wvsprintf( buff+lstrlen(buff), fmt, (char*)(&fmt+1) );
    lstrcat( buff, "\r\n" );

    OutputDebugString( buff );
}

#endif

//----------- End of function Msg ----------//


//------- Begin of function extra_error_handler -----------//

static void extra_error_handler()
{
   if( game.game_mode != GAME_SINGLE_PLAYER )
      return;

	game_file_array.save_new_game("ERROR.SAV");  // save a new game immediately without prompting menu

   box.msg( "Error encountered. The game has been saved to ERROR.SAV" );
}
//----------- End of function extra_error_handler -------------//
