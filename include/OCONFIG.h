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

//Filename	  : OCONFIG.H
//Description : Header file for Game Config class

#ifndef __OCONFIG_H
#define __OCONFIG_H

#ifndef __OFILE_H
#include <OFILE.h>
#endif

//------------- Define constant -------------//

enum { OPTION_NONE=0, OPTION_LOW, OPTION_MODERATE, OPTION_HIGH, OPTION_VERY_HIGH };

enum { OPTION_MONSTER_NONE=0, OPTION_MONSTER_DEFENSIVE, OPTION_MONSTER_OFFENSIVE };

enum { OPTION_VERY_EASY, OPTION_EASY, OPTION_MEDIUM, OPTION_HARD, OPTION_VERY_HARD, OPTION_CUSTOM };

enum { OPTION_DISPLAY_MAJOR_NEWS, OPTION_DISPLAY_ALL_NEWS };

//------------- Define constant -------------//

enum { SMALL_STARTUP_RESOURCE  = 4000,
		 MEDIUM_STARTUP_RESOURCE = 7000,
		 LARGE_STARTUP_RESOURCE  = 12000,
		 VERY_LARGE_STARTUP_RESOURCE = 20000 };

//---------- Define class Config -----------//

#pragma pack(1)
class Config
{
public:
	void			init();
	void			deinit();

	void			default_game_setting();
	void			default_cheat_setting();
	void			default_local_game_setting();
	void			default_preference();
	void			change_game_setting( Config & );
	void			change_preference( Config & );
	void			change_difficulty(int);

	int			single_player_difficulty();
	int			multi_player_difficulty(int remotePlayers);

	int 			write_file(File* filePtr);
	int			read_file(File* filePtr, int keepSysSettings=0);
	int			save(char *);		// save to file
	int			load(char *);		// load from file

	void			reset_cheat_setting();
	void			enable_weather_visual();
	void			disable_weather_visual();
	void			enable_weather_audio();
	void			disable_weather_audio();

public:
	enum { PLAYER_NAME_LEN=20 };		// should be te same as NationBase::KING_NAME_LEN

	//--------- GLOBAL GAME SETTING --------//
	//
	// parameters under GLOBAL GAME SETTING should remain unchange
	// after the game starts, and are the same across other players
	// in a multiplayer game
	// (i.e. change_game_setting() should updates all these setting )
	//
	//--------------------------------------//

	//------- parameter settings --------//

	short			difficulty_rating;

	char			ai_nation_count;			// no. of AI nations in the game
	short			start_up_cash;
//	short			start_up_food;
	short			ai_start_up_cash;
//	short			ai_start_up_food;
	char			ai_aggressiveness;
	short			start_up_independent_town;
	short			start_up_raw_site;
	char			difficulty_level;

	//-------- option settings  ---------//

	char			explore_whole_map;			// whether the map is explored at first place
	char			fog_of_war;						// fog of war option

	short			terrain_set;
	short			latitude;
	char			weather_effect;
	char			land_mass;

	char			new_independent_town_emerge;
	char		 	independent_town_resistance; 	// whether independent towns' defenders have higher combat levels
	char			random_event_frequency;
	char			new_nation_emerge;
	char			monster_type;
	char			start_up_has_mine_nearby;
	char			random_start_up;

	//--------- goal ----------//

	char			goal_destroy_monster;
	char			goal_population_flag;
	char			goal_economic_score_flag;
	char			goal_total_score_flag;
	char			goal_year_limit_flag;

	int			goal_population;
	int  			goal_economic_score;
	int			goal_total_score;
	int			goal_year_limit;

	//------- game setting on fire ---------//

	char			fire_spread_rate;          // 0 to disable, 10 for normal
	char			wind_spread_fire_rate;     // 0 to disable, 5 for normal
	char			fire_fade_rate;            // 1 for slow, 2 for fast
	char			fire_restore_prob;         // 0 to 100, 5 for normal
	char			rain_reduce_fire_rate;     // 0 to 20, 5 for normal
	char			fire_damage;               // 0 to disable 2 for normal

	//--------- CHEAT GAME SETTING --------//
	//
	// parameters under CHEAT GAME SETTING can be changed
	// after the game starts, and must be reset in in a multiplayer game
	// (i.e. reset_cheat_setting() can reset all these setting )
	//
	//--------------------------------------//
	char			show_ai_info;
	char			fast_build;							// fast building everything
	char			disable_ai_flag;
	char			king_undie_flag;					// for testing game only
	
	//--------- LOCAL GAME SETTING --------//
	//
	// parameters under LOCAL GAME SETTING should remain unchange
	// after the game starts, may not be the same across other players
	//
	//-------------------------------------//

	char			race_id;
	char			player_name[PLAYER_NAME_LEN+1];
	char			player_nation_color;

	char			expired_flag;

	//--------- PREFERENCE --------//
	//
	// parameters under PREFERENCE are changeable during the game
	// the game will not be affect at any setting
	//
	//-------------------------------------//

	char			opaque_report;
	char			disp_news_flag;

	short			scroll_speed;					// map scrolling speed. 1-slowest, 10-fastest
	short			frame_speed;					// game speed, the desired frames per second

	char			help_mode;
	char			disp_town_name;
	char			disp_spy_sign;
	char			show_all_unit_icon;			// 0:show icon when pointed, 1:always
	char			show_unit_path;				// bit 0 show unit path on ZoomMatrix, bit 1 for MapMatrix

	//------- sound effect --------//

	char			music_flag;
	short			cd_music_volume;    	// a value from 0 to 100
	short			wav_music_volume;		// a value from 0 to 100

	char			sound_effect_flag;
	short			sound_effect_volume; 	// a value from 0 to 100

	char			pan_control;                            // mono speaker should disable pan_control

	//------- weather visual effect flags -------//

	char			lightning_visual;
	char			earthquake_visual;
	char			rain_visual;
	char			snow_visual;
	char			snow_ground;			// 0=disable, 1=i_snow, 2=snow_res

	//-------- weather audio effect flags -------//

	char			lightning_audio;
	char			earthquake_audio;
	char			rain_audio;
	char			snow_audio;				// not used
	char			wind_audio;

	//--------- weather visual effect parameters --------//

	int			lightning_brightness;	// 0, 20, 40 or 60
	int			cloud_darkness;		// 0 to 5, 0 to disable cloud darkness

	//-------- weather audio effect parameters ----------//

	long			lightning_volume;		// default 100
	long			earthquake_volume;	// default 100
	long			rain_volume;			// default 90
	long			snow_volume;			// default 100
	long			wind_volume;			// default 70

	//------------ map prefernce -------------//

	char			blacken_map;				// whether the map is blackened at the first place
	char			explore_mask_method;		// 0 for none, 1 for masking, 2 for remapping
	char			fog_mask_method;			// 1 for fast masking, 2 for slow remapping
};
#pragma pack()

//------------------------------------------//

extern Config config;

#endif
