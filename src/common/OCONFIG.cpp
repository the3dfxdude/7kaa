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

//Filename    : OCONFIG.CPP
//Description : Config Object

#include <OSYS.h>
#include <OHELP.h>
#include <ONATION.h>
#include <OUNITRES.h>
#include <OFIRMRES.h>
#include <OCONFIG.h>


// ------- define difficult table ---------//

static char  table_ai_nation_count[5] = { 2, 4, 6, 6, 6 };
static short table_start_up_cash[5] = { OPTION_HIGH, OPTION_HIGH, OPTION_MODERATE, OPTION_MODERATE, OPTION_LOW };
static short table_ai_start_up_cash[5] = { OPTION_LOW, OPTION_MODERATE, OPTION_MODERATE, OPTION_HIGH, OPTION_VERY_HIGH };
static char  table_ai_aggressiveness[5] = { OPTION_LOW, OPTION_LOW, OPTION_MODERATE, OPTION_HIGH, OPTION_VERY_HIGH };
static short table_start_up_independent_town[5] = { 30, 30, 15, 15, 7 };
static short table_start_up_raw_site[5] = { 7, 6, 6, 4, 3, };
static char  table_explore_whole_map[5] = { 1, 1, 1, 1, 0 };
static char  table_fog_of_war[5] = { 0, 0, 0, 1, 1 };
static char  table_new_independent_town_emerge[5] = { 1, 1, 1, 1, 1 };
static char  table_independent_town_resistance[5] = { OPTION_LOW, OPTION_LOW, OPTION_MODERATE, OPTION_HIGH, OPTION_HIGH };
static char  table_random_event_frequency[5] = { OPTION_NONE, OPTION_LOW, OPTION_MODERATE, OPTION_MODERATE, OPTION_MODERATE };
static char  table_new_nation_emerge[5] = { 0, 0, 0, 1, 1 };
static char  table_monster_type[5] = { OPTION_MONSTER_DEFENSIVE, OPTION_MONSTER_DEFENSIVE, OPTION_MONSTER_DEFENSIVE, OPTION_MONSTER_OFFENSIVE, OPTION_MONSTER_OFFENSIVE };
static char  table_start_up_has_mine_nearby[5] = { 1, 1, 0, 0, 0 };
static char  table_random_start_up[5] = { 0, 0, 0, 1, 1 };

//--------- Begin of function Config::init -----------//

void Config::init()
{
	default_game_setting();
	default_cheat_setting();
	default_local_game_setting();
	default_preference();
}
//--------- End of function Config::init --------//


//--------- Begin of function Config::deinit -----------//

void Config::deinit()
{
	config.save("CONFIG.DAT");		// save the config when the game quits

	default_game_setting();
	default_local_game_setting();
	default_preference();
}
//--------- End of function Config::deinit --------//


//--------- Begin of function Config::default_game_setting ---------//
void Config::default_game_setting()
{
	// -------- GLOBAL GAME SETTING -------- //

	ai_nation_count = 6 ;
	start_up_cash = OPTION_MODERATE;
//	start_up_food = MEDIUM_STARTUP_RESOURCE;
	ai_start_up_cash = OPTION_MODERATE;
//	ai_start_up_food = MEDIUM_STARTUP_RESOURCE;
	ai_aggressiveness = OPTION_MODERATE;
	start_up_independent_town = 15;
	start_up_raw_site = 6;
	difficulty_level = OPTION_CUSTOM;

	explore_whole_map = 1;
	fog_of_war   = 0;

	terrain_set = 1;
	latitude = 45;
	weather_effect = 1;		// damage done by weather
	land_mass = OPTION_MODERATE;

	new_independent_town_emerge = 1;
	independent_town_resistance = OPTION_MODERATE;
	random_event_frequency = OPTION_NONE;
	monster_type = OPTION_MONSTER_DEFENSIVE;
	new_nation_emerge = 1;
	start_up_has_mine_nearby = 0;
   random_start_up = 0;

	change_difficulty(OPTION_VERY_EASY);

	//-------- goal --------//

	goal_destroy_monster = 0;
	goal_population_flag = 0;
	goal_economic_score_flag = 0;
	goal_total_score_flag = 0;
	goal_year_limit_flag = 0;

	goal_population = 300;
	goal_economic_score = 300;
	goal_total_score = 600;
	goal_year_limit = 20;

	// game setting on fire
	fire_spread_rate = 0;          // 0 to disable, 10 for normal
	wind_spread_fire_rate = 5;     // 0 to disable, 5 for normal
	fire_fade_rate = 2;            // 1 for slow, 2 for fast
	fire_restore_prob = 80;        // 0 to 100, 5 for normal (with spreading) 
	rain_reduce_fire_rate = 5;     // 0 to 20, 5 for normal
	fire_damage = 2;               // 0 to disable 2 for normal
}
//--------- End of function Config::default_game_setting ---------//


//--------- Begin of function Config::default_game_setting ---------//
void Config::default_cheat_setting()
{
	show_ai_info = sys.debug_session;
	fast_build = sys.debug_session;
	disable_ai_flag = 0;
	king_undie_flag = sys.testing_session;
}
//--------- End of function Config::default_game_setting ---------//


//--------- Begin of function Config::default_local_game_setting ---------//
void Config::default_local_game_setting()
{
	race_id = 1;
#if(defined(SPANISH))
	strcpy(player_name, "Nuevo Jugador");
#elif(defined(FRENCH))
	strcpy(player_name, "Nouveau Joueur");
#elif(defined(GERMAN))
	strcpy(player_name, "Neuer Spieler");
#else
	strcpy(player_name, "New Player");
#endif
	player_nation_color = 1;
	expired_flag = 0;
}
//--------- End of function Config::default_local_game_setting ---------//


//--------- Begin of function Config::default_preference ---------//

void Config::default_preference()
{
	opaque_report = 0;			// opaque report instead of transparent report
	disp_news_flag = OPTION_DISPLAY_ALL_NEWS;

	scroll_speed = 5;
	frame_speed	 = 12;

	help_mode = DETAIL_HELP;
	disp_town_name = 1;
	disp_spy_sign = 1;
	show_all_unit_icon = 1;		// 0:show icon when pointed, 1:always
	show_unit_path = 3;			// bit 0 show unit path on ZoomMatrix, bit 1 for MapMatrix

	// music setting
	music_flag = 1;
	cd_music_volume = 100;
	wav_music_volume = 100;

	// sound effect setting
	sound_effect_flag = 1;
	sound_effect_volume = 100;
	pan_control = 1;

	// weather visual effect flags
	lightning_visual = 1;
	earthquake_visual = 1;
	rain_visual = 1;
	snow_visual = 1;
	snow_ground = 0;			// 0=disable, 1=i_snow, 2=snow_res

	// weather audio effect flags
	lightning_audio = 1;
	earthquake_audio = 1;
	rain_audio = 1;
	snow_audio = 0;				// not used
	wind_audio = 1;

	// weather visual effect parameters
	lightning_brightness = 20;
	cloud_darkness = 5;

	// weather audio effect parameters
	lightning_volume = 100;
	earthquake_volume = 100;
	rain_volume = 90;
	snow_volume = 100;
	wind_volume = 70;

	// other
	blacken_map  = 1;
	explore_mask_method = 2;
	fog_mask_method = 2;

	enable_weather_visual();
	enable_weather_audio();
	cloud_darkness = 0;
}
//--------- End of function Config::default_preference ---------//


//--------- Begin of function Config::change_game_setting --------//
// for synchronize the game setting with the host before a multiplayer game
void Config::change_game_setting( Config &c )
{
	//-------- game settings  ---------//
	ai_nation_count        = c.ai_nation_count;
	start_up_cash          = c.start_up_cash;
	// start_up_food          = c.start_up_food;
	ai_start_up_cash       = c.ai_start_up_cash;
	// ai_start_up_food       = c.ai_start_up_food;
	ai_aggressiveness      = c.ai_aggressiveness;
	start_up_independent_town = c.start_up_independent_town;
	start_up_raw_site      = c.start_up_raw_site;
	difficulty_level       = c.difficulty_level;

	explore_whole_map      = c.explore_whole_map;
	fog_of_war             = c.fog_of_war;

	terrain_set            = c.terrain_set;
	latitude               = c.latitude;
	weather_effect         = c.weather_effect;
	land_mass              = c.land_mass;

	new_independent_town_emerge = c.new_independent_town_emerge;
	independent_town_resistance = c.independent_town_resistance;
	random_event_frequency = c.random_event_frequency;
	monster_type           = c.monster_type;
	new_nation_emerge      = c.new_nation_emerge;
	start_up_has_mine_nearby = c.start_up_has_mine_nearby;
	random_start_up 		  = c.random_start_up;

	// --------- goal ---------//

	goal_destroy_monster      = c.goal_destroy_monster;
	goal_population_flag      = c.goal_population_flag;
	goal_economic_score_flag  = c.goal_economic_score_flag;
	goal_total_score_flag  	  = c.goal_total_score_flag;
	goal_year_limit_flag   	  = c.goal_year_limit_flag;
	goal_population        	  = c.goal_population;
	goal_economic_score       = c.goal_economic_score;
	goal_total_score       	  = c.goal_total_score;
	goal_year_limit        	  = c.goal_year_limit;

	// ------- game setting on fire ---------//

	fire_spread_rate       = c.fire_spread_rate;
	wind_spread_fire_rate  = c.wind_spread_fire_rate;
	fire_fade_rate         = c.fire_fade_rate;
	fire_restore_prob      = c.fire_restore_prob;
	rain_reduce_fire_rate  = c.rain_reduce_fire_rate;
	fire_damage            = c.fire_damage;
}
//--------- End of function Config::change_game_setting --------//


//--------- Begin of function Config::change_game_setting --------//
// for saving config after changing option
void Config::change_preference( Config &c )
{
	opaque_report         = c.opaque_report;
	disp_news_flag        = c.disp_news_flag;

	scroll_speed          = c.scroll_speed;
	frame_speed           = c.frame_speed;

	help_mode             = c.help_mode;
	disp_town_name        = c.disp_town_name;
	disp_spy_sign         = c.disp_spy_sign;
	show_all_unit_icon    = c.show_all_unit_icon;
	show_unit_path        = c.show_unit_path;

	//------- sound effect --------//

	music_flag            = c.music_flag;
	cd_music_volume       = c.cd_music_volume;
	wav_music_volume      = c.wav_music_volume;
	sound_effect_flag     = c.sound_effect_flag;
	sound_effect_volume   = c.sound_effect_volume;
	pan_control           = c.pan_control;

	//------- weather visual effect flags -------//

	lightning_visual      = c.lightning_visual;
	earthquake_visual     = c.earthquake_visual;
	rain_visual           = c.rain_visual;
	snow_visual           = c.snow_visual;
	snow_ground           = c.snow_ground;

	//-------- weather audio effect flags -------//

	lightning_audio       = c.lightning_audio;
	earthquake_audio      = c.earthquake_audio;
	rain_audio            = c.rain_audio;
	snow_audio            = c.snow_audio;
	wind_audio            = c.wind_audio;

	//--------- weather visual effect parameters --------//

	lightning_brightness  = c.lightning_brightness;
	cloud_darkness        = c.cloud_darkness;

	//-------- weather audio effect parameters ----------//

	lightning_volume = c.lightning_volume;
	earthquake_volume = c.earthquake_volume;
	rain_volume = c.rain_volume;
	snow_volume = c.snow_volume;
	wind_volume = c.wind_volume;

	//------------ map prefernce -------------//

	blacken_map = c.blacken_map;
	explore_mask_method = c.explore_mask_method;
	fog_mask_method = c.fog_mask_method;
}


//--------- Begin of function Config::enable_weather_visual --------//
void Config::enable_weather_visual()
{
	lightning_visual = 1;
	earthquake_visual = 1;
	rain_visual = 1;
	snow_visual = 1;
	snow_ground = 0;			// 0=disable, 1=i_snow, 2=snow_res
	cloud_darkness = 1;
}
//--------- End of function Config::enable_weather_visual --------//


//--------- Begin of function Config::disable_weather_visual --------//
void Config::disable_weather_visual()
{
	lightning_visual = 0;
	earthquake_visual = 0;
	rain_visual = 0;
	snow_visual = 0;
	snow_ground = 0;			// 0=disable, 1=i_snow, 2=snow_res
	cloud_darkness = 0;
}
//--------- End of function Config::disable_weather_visual --------//


//--------- Begin of function Config::enable_weather_audio --------//
void Config::enable_weather_audio()
{
	lightning_audio = 1;
	earthquake_audio = 1;
	rain_audio = 1;
	snow_audio = 1;				// not used
	wind_audio = 1;
}
//--------- End of function Config::enable_weather_audio --------//


//--------- Begin of function Config::disable_weather_audio --------//
void Config::disable_weather_audio()
{
	lightning_audio = 0;
	earthquake_audio = 0;
	rain_audio = 0;
	snow_audio = 0;				// not used
	wind_audio = 0;
}
//--------- End of function Config::disable_weather_audio --------//


//--------- Begin of function Config::save -------------//
int Config::save(const char *filename)
{
	File configFile;

	if( !configFile.file_create(filename) )
		return 0;

	int retFlag = write_file(&configFile);

	configFile.file_close();

	return retFlag;

}
//--------- End of function Config::save -------------//


//--------- Begin of function Config::load -------------//
//
// if load() fails, call init to re-initialize it
//
int Config::load(const char *filename)
{
	File configFile;

	if( !m.is_file_exist(filename) || !configFile.file_open(filename) )
		return 0;

	int retFlag = 0;

	// check file size is the same
	if( configFile.file_size() == sizeof(Config) )
	{
		retFlag = read_file(&configFile);
	}

	configFile.file_close();

	return retFlag;
}
//--------- End of function Config::load -------------//


//--------- Begin of function Config::reset_game_setting ---------//
void Config::reset_cheat_setting()
{
	show_ai_info = 0;
	fast_build = 0;
	disable_ai_flag = 0;
	king_undie_flag = 0;
}
//--------- End of function Config::reset_game_setting ---------//


//------- Begin of function Config::single_player_difficulty --------//
int Config::single_player_difficulty()
{
	int score = 10;
	score += ai_nation_count * 6;
	if( !explore_whole_map)
		score += 7;
	if( fog_of_war )
		score += 7;
	score += (7 - start_up_raw_site) * 5;

//	if( start_up_cash <= SMALL_STARTUP_RESOURCE )
//		score += 16;
//	else if( start_up_cash < LARGE_STARTUP_RESOURCE )
//		score += 8;
//	else
//		score += 0;
	score += (4 - start_up_cash) * 6;

//	if( ai_start_up_cash <= SMALL_STARTUP_RESOURCE )
//		score += 0;
//	else if( ai_start_up_cash < LARGE_STARTUP_RESOURCE )
//		score += 8;
//	else
//		score += 16;
	score += (ai_start_up_cash - 1 ) * 6;

	score += (ai_aggressiveness -1) * 10;
	score += (independent_town_resistance -1) * 5;
	if( new_nation_emerge )
		score += 6;
	score += random_event_frequency * 2;
	switch( monster_type )
	{
	case OPTION_MONSTER_NONE:
		break;
	case OPTION_MONSTER_DEFENSIVE:
		score += 6;
		break;
	case OPTION_MONSTER_OFFENSIVE:
		score += 16;
		break;
	default:
		err_here();
	}

	if( !start_up_has_mine_nearby )
		score += 6;
	return score;
}
//------- End of function Config::single_player_difficulty --------//


//------- Begin of function Config::multi_player_difficulty --------//
int Config::multi_player_difficulty(int remotePlayers)
{
	short totalOpp = ai_nation_count + remotePlayers;
	if( totalOpp > MAX_NATION-1 )
		totalOpp = MAX_NATION-1;
	return single_player_difficulty() + (totalOpp - ai_nation_count) * 6;
}
//------- End of function Config::multi_player_difficulty --------//


//------- Begin of function Config::multi_player_difficulty --------//
void Config::change_difficulty(int difficulty)
{
	err_when( difficulty < OPTION_VERY_EASY || difficulty > OPTION_VERY_HARD );
	
	difficulty_level = difficulty;

	ai_nation_count             = table_ai_nation_count[difficulty];
	start_up_cash               = table_start_up_cash[difficulty];
	// start_up_food               = table_start_up_food[difficulty];
	ai_start_up_cash            = table_ai_start_up_cash[difficulty];
	// ai_start_up_food            = table_ai_start_up_food[difficulty];
	ai_aggressiveness           = table_ai_aggressiveness[difficulty];
	start_up_independent_town   = table_start_up_independent_town[difficulty];
	start_up_raw_site           = table_start_up_raw_site[difficulty];
	explore_whole_map           = table_explore_whole_map[difficulty];
	fog_of_war                  = table_fog_of_war[difficulty];

	new_independent_town_emerge = table_new_independent_town_emerge[difficulty];
	independent_town_resistance = table_independent_town_resistance[difficulty];
	random_event_frequency      = table_random_event_frequency[difficulty];
	new_nation_emerge           = table_new_nation_emerge[difficulty];
	monster_type                = table_monster_type[difficulty];
	start_up_has_mine_nearby    = table_start_up_has_mine_nearby[difficulty];
	random_start_up    			 = table_random_start_up[difficulty];
}
//------- End of function Config::change_difficulty --------//

